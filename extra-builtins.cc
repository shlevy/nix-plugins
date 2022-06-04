#include <config.h>
#include <primops.hh>
#include <globals.hh>
#include <eval-inline.hh>
#include <dlfcn.h>

#include "nix-plugins-config.h"

#if HAVE_BOEHMGC

#include <gc/gc.h>
#include <gc/gc_cpp.h>

#endif


using namespace nix;

struct ExtraBuiltinsSettings : Config {
    Setting<Path> extraBuiltinsFile{this,
        settings.nixConfDir + "/extra-builtins.nix",
            "extra-builtins-file",
            "The path to a nix expression defining extra expression-language level builtins."};
};

static ExtraBuiltinsSettings extraBuiltinsSettings;

static GlobalConfig::Register rp(&extraBuiltinsSettings);

static void extraBuiltins(EvalState & state, const PosIdx pos,
    Value ** _args, Value & v)
{
    static auto extraBuiltinsFile = absPath(extraBuiltinsSettings.extraBuiltinsFile);
    if (state.allowedPaths)
        state.allowedPaths->insert(extraBuiltinsFile);

    try {
        auto fun = state.allocValue();
        state.evalFile(extraBuiltinsFile, *fun);
        Value * arg;
        if (evalSettings.enableNativeCode) {
            arg = state.baseEnv.values[0];
        } else {
            auto attrs = state.buildBindings(2);

            auto sExec = state.symbols.create("exec");
            attrs.alloc(sExec).mkPrimOp(new PrimOp { .fun = prim_exec, .arity = 1, .name = "exec" });

            auto sImportNative = state.symbols.create("importNative");
            attrs.alloc(sImportNative).mkPrimOp(new PrimOp { .fun = prim_importNative, .arity = 2, .name = "importNative" });

            arg = state.allocValue();
            arg->mkAttrs(attrs);
        }
        v.mkApp(fun, arg);
        state.forceValue(v, pos);
    } catch (SysError & e) {
        if (e.errNo != ENOENT)
            throw;
        v.mkNull();
    }
}

static RegisterPrimOp rp1("__extraBuiltins", 0,
    extraBuiltins);

static void cflags(EvalState & state, const PosIdx _pos,
    Value ** _args, Value & v)
{
    auto attrs = state.buildBindings(3);
    attrs.alloc("NIX_INCLUDE_DIRS").mkString(NIX_INCLUDE_DIRS);
    attrs.alloc("NIX_CFLAGS_OTHER").mkString(NIX_CFLAGS_OTHER);
    attrs.alloc("BOOST_INCLUDE_DIR").mkString(BOOST_INCLUDE_DIR);
    v.mkAttrs(attrs);
}

static RegisterPrimOp rp2("__nix-cflags", 0,
    cflags);
