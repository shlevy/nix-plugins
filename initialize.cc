#include <gc/gc_cpp.h>
#include "primops.hpp"

static void addPrimOp(nix::EvalState& state, nix::Value& v, const nix::Symbol & name, unsigned int arity, nix::PrimOpFun primOp) {
    nix::Value* vAttr = state.allocAttr(v, name);
    vAttr->type = nix::tPrimOp;
    vAttr->primOp = new (UseGC) nix::PrimOp(primOp, arity, name);
}

extern "C" [[gnu::visibility ("default")]]
    void initialize(nix::EvalState& state, nix::Value& v) {
    state.mkAttrs(v, 1);
    addPrimOp(state, v, state.symbols.create("exec"), 1, prim_exec);
}
