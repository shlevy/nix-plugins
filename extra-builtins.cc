#include <config.h>
#include <primops.hh>
#include <globals.hh>
#include <eval-inline.hh>
#include <dlfcn.h>

#include "nix-plugins-config.h"

#if HAVE_BOEHMGC

#include <gc/gc.h>
#include <gc/gc_cpp.h>

#define NEW new (UseGC)

#else

#define NEW new

#endif


using namespace nix;

static BaseSetting<Path> extraBuiltinsFile{
    settings.nixConfDir + "/extra-builtins.nix",
        "extra-builtins-file",
        "The path to a nix expression defining extra expression-language level builtins."};

static RegisterSetting rp(&extraBuiltinsFile);


static void extraBuiltins(EvalState & state, const Pos & _pos,
    Value ** _args, Value & v)
{
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

static RegisterPrimOp rp1("__extraBuiltins", 0,
    extraBuiltins);

static void cflags(EvalState & state, const Pos & _pos,
    Value ** _args, Value & v)
{
    state.mkAttrs(v, 3);
    mkStringNoCopy(*state.allocAttr(v, state.symbols.create("NIX_INCLUDE_DIRS")), NIX_INCLUDE_DIRS);
    mkStringNoCopy(*state.allocAttr(v, state.symbols.create("NIX_CFLAGS_OTHER")), NIX_CFLAGS_OTHER);
    mkStringNoCopy(*state.allocAttr(v, state.symbols.create("BOOST_INCLUDE_DIR")), BOOST_INCLUDE_DIR);
}

static RegisterPrimOp rp2("__nix-cflags", 0,
    cflags);
