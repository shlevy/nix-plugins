#include <nix/eval.hh>

[[gnu::visibility ("default")]]
    extern "C" void initialize(nix::EvalState& state, nix::Value& v) {
    state.mkAttrs(v, 0);
}
