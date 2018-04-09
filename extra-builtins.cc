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

static void cflags(EvalState & state, const Pos & _pos,
    Value ** _args, Value & v)
{
    state.mkAttrs(v, 2);
    mkStringNoCopy(*state.allocAttr(v, state.symbols.create("NIX_INCLUDE_DIRS")), NIX_INCLUDE_DIRS);
    mkStringNoCopy(*state.allocAttr(v, state.symbols.create("NIX_CFLAGS_OTHER")), NIX_CFLAGS_OTHER);
}

static RegisterPrimOp r1("__com.shealevy.nix-plugins.nix-cflags", 0,
    cflags);
