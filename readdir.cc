#include <nix/util.hh>
#include "primops.hpp"

[[gnu::visibility ("hidden")]]
    void prim_readdir(nix::EvalState& state, const nix::Pos& pos, nix::Value** args, nix::Value& v) {
    nix::PathSet context;
    nix::Path path = state.coerceToPath(pos, *args[0], context);
    if (!context.empty())
        throw nix::EvalError(nix::format("string `%1' cannot refer to other paths") % path);
    nix::Strings entries = nix::readDirectory(path);
    state.mkList(v, entries.size());
    entries.sort();
    unsigned int index = 0;
    for (const auto& i : entries)
        nix::mkString(*(v.list.elems[index++] = state.allocValue()), i);
}
