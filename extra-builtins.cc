#include <config.h>
#include <primops.hh>
#include <globals.hh>
#include <config-global.hh>
#include <eval-settings.hh>
#include <common-eval-args.hh>
#include <filtering-source-accessor.hh>

#include "nix-plugins-config.h"

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
    static auto extraBuiltinsFile = state.rootPath(CanonPath(extraBuiltinsSettings.extraBuiltinsFile.to_string()));
    if (auto rootFS2 = state.rootFS.dynamic_pointer_cast<AllowListSourceAccessor>())
        rootFS2->allowPrefix(CanonPath(extraBuiltinsFile.path.abs()));

    try {
        auto fun = state.allocValue();
        state.evalFile(extraBuiltinsFile, *fun);
        Value * arg;
        if (evalSettings.enableNativeCode) {
            arg = state.baseEnv.values[0];
        } else {
            auto attrs = state.buildBindings(2);

            auto sExec = state.symbols.create("exec");
            attrs.alloc(sExec).mkPrimOp(new PrimOp {
                .name = "exec",
                .arity = 1,
                .fun = prim_exec,
            });

            auto sImportNative = state.symbols.create("importNative");
            attrs.alloc(sImportNative).mkPrimOp(new PrimOp {
                .name = "importNative",
                .arity = 2,
                .fun = prim_importNative,
            });

            arg = state.allocValue();
            arg->mkAttrs(attrs);
        }
        v.mkApp(fun, arg);
        state.forceValue(v, pos);
    } catch (FileNotFound &) {
        v.mkNull();
    }
}

static RegisterPrimOp rp1({
    .name = "__extraBuiltins",
    .arity = 0,
    .fun = extraBuiltins,
});

static void cflags(EvalState & state, const PosIdx _pos,
    Value ** _args, Value & v)
{
    auto attrs = state.buildBindings(3);
    attrs.alloc("NIX_INCLUDE_DIRS").mkString(NIX_INCLUDE_DIRS);
    attrs.alloc("NIX_CFLAGS_OTHER").mkString(NIX_CFLAGS_OTHER);
    attrs.alloc("BOOST_INCLUDE_DIR").mkString(BOOST_INCLUDE_DIR);
    v.mkAttrs(attrs);
}

static RegisterPrimOp rp2({
    .name = "__nix-cflags",
    .arity = 0,
    .fun = cflags,
});
