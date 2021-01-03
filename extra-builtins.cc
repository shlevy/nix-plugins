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
        fmt("%s/%s", settings.nixConfDir, "extra-builtins.nix"),
            "extra-builtins-file",
            "The path to a nix expression defining extra expression-language level builtins."};
};

static ExtraBuiltinsSettings extraBuiltinsSettings;

static GlobalConfig::Register rp(&extraBuiltinsSettings);

static void extraBuiltins(EvalState & state, const Pos & _pos,
    Value ** _args, Value & v)
{
    static auto extraBuiltinsFile = extraBuiltinsSettings.extraBuiltinsFile;
    if (state.allowedPaths)
        state.allowedPaths->insert(extraBuiltinsFile);

    try {
        auto fun = state.allocValue();
        state.evalFile(extraBuiltinsFile, *fun);
        Value * arg;
        if (evalSettings.enableNativeCode) {
            arg = state.baseEnv.values[0];
        } else {
            arg = state.allocValue();
            state.mkAttrs(*arg, 2);

            auto sExec = state.symbols.create("exec");
            auto vExec = state.allocAttr(*arg, sExec);
            vExec->mkPrimOp(new PrimOp { .fun = prim_exec, .arity = 1, .name = sExec});

            auto sImportNative = state.symbols.create("importNative");
            auto vImportNative = state.allocAttr(*arg, sImportNative);
            vImportNative->mkPrimOp(new PrimOp { .fun = prim_importNative, .arity = 2, .name = sImportNative });

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
