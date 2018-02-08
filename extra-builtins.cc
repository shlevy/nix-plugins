#include <nix/config.h>
#include <nix/primops.hh>
#include <nix/globals.hh>
#include <dlfcn.h>

#if HAVE_BOEHMGC

#include <gc/gc.h>
#include <gc/gc_cpp.h>

#define NEW new (UseGC)

#else

#define NEW new

#endif


using namespace nix;

/* prim_importNative and prim_exec taken from upstream */

/* Want reasonable symbol names, so extern C */
/* !!! Should we pass the Pos or the file name too? */
extern "C" typedef void (*ValueInitializer)(EvalState & state, Value & v);

/* Load a ValueInitializer from a DSO and return whatever it initializes */
static void prim_importNative(EvalState & state, const Pos & pos, Value * * args, Value & v)
{
    PathSet context;
    Path path = state.coerceToPath(pos, *args[0], context);

    try {
        state.realiseContext(context);
    } catch (InvalidPathError & e) {
        throw EvalError(format("cannot import '%1%', since path '%2%' is not valid, at %3%")
            % path % e.path % pos);
    }

    path = state.checkSourcePath(path);

    string sym = state.forceStringNoCtx(*args[1], pos);

    void *handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (!handle)
        throw EvalError(format("could not open '%1%': %2%") % path % dlerror());

    dlerror();
    ValueInitializer func = (ValueInitializer) dlsym(handle, sym.c_str());
    if(!func) {
        char *message = dlerror();
        if (message)
            throw EvalError(format("could not load symbol '%1%' from '%2%': %3%") % sym % path % message);
        else
            throw EvalError(format("symbol '%1%' from '%2%' resolved to NULL when a function pointer was expected")
                    % sym % path);
    }

    (func)(state, v);

    /* We don't dlclose because v may be a primop referencing a function in the shared object file */
}


/* Execute a program and parse its output */
static void prim_exec(EvalState & state, const Pos & pos, Value * * args, Value & v)
{
    state.forceList(*args[0], pos);
    auto elems = args[0]->listElems();
    auto count = args[0]->listSize();
    if (count == 0) {
        throw EvalError(format("at least one argument to 'exec' required, at %1%") % pos);
    }
    PathSet context;
    auto program = state.coerceToString(pos, *elems[0], context, false, false);
    Strings commandArgs;
    for (unsigned int i = 1; i < args[0]->listSize(); ++i) {
        commandArgs.emplace_back(state.coerceToString(pos, *elems[i], context, false, false));
    }
    try {
        state.realiseContext(context);
    } catch (InvalidPathError & e) {
        throw EvalError(format("cannot execute '%1%', since path '%2%' is not valid, at %3%")
            % program % e.path % pos);
    }

    auto output = runProgram(program, true, commandArgs);
    Expr * parsed;
    try {
        parsed = state.parseExprFromString(output, pos.file);
    } catch (Error & e) {
        e.addPrefix(format("While parsing the output from '%1%', at %2%\n") % program % pos);
        throw;
    }
    try {
        state.eval(parsed, v);
    } catch (Error & e) {
        e.addPrefix(format("While evaluating the output from '%1%', at %2%\n") % program % pos);
        throw;
    }
}

static void extraBuiltins(EvalState & state, const Pos & _pos,
    Value ** _args, Value & v)
{
    auto extraBuiltinsFile =
        absPath(settings.nixConfDir + "/extra-builtins.nix");
    if (state.allowedPaths)
        state.allowedPaths->insert(extraBuiltinsFile);
    
    try {
        auto fun = state.allocValue();
        state.evalFile(extraBuiltinsFile, *fun);
        Value * arg;
        if (settings.enableNativeCode) {
            arg = state.baseEnv.values[0];
        } else {
            arg = state.allocValue();
            state.mkAttrs(*arg, 2);

            auto sExec = state.symbols.create("exec");
            auto vExec = state.allocAttr(*arg, sExec);
            vExec->type = tPrimOp;
            vExec->primOp = NEW PrimOp(prim_exec, 1, sExec);

            auto sImportNative = state.symbols.create("importNative");
            auto vImportNative = state.allocAttr(*arg, sImportNative);
            vImportNative->type = tPrimOp;
            vImportNative->primOp = NEW PrimOp(prim_importNative, 2, sImportNative);

            arg->attrs->sort();
        }
        mkApp(v, *fun, *arg);
    } catch (SysError & e) {
        if (e.errNo != ENOENT)
            throw;
        mkNull(v);
    }
}

static RegisterPrimOp r("__com.shealevy.nix-plugins.extraBuiltins", 0,
    extraBuiltins);
