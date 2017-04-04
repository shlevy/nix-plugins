#include <nix/eval.hh>
#include <nix/eval-inline.hh>
using namespace nix;

#ifndef ENABLE_S3
void nix::realiseContext(const PathSet & context);
#endif

[[gnu::visibility ("hidden")]]
    void prim_exec(EvalState & state, const Pos & pos, Value * * args, Value & v)
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
#ifdef ENABLE_S3
        state.realiseContext(context);
#else
        realiseContext(context);
#endif
    } catch (InvalidPathError & e) {
        throw EvalError(format("cannot execute ‘%1%’, since path ‘%2%’ is not valid, at %3%")
            % program % e.path % pos);
    }

    auto output = runProgram(program, true, commandArgs);
    Expr * parsed;
    try {
        parsed = state.parseExprFromString(output, pos.file);
    } catch (Error & e) {
        e.addPrefix(format("While parsing the output from ‘%1%’, at %2%\n") % program % pos);
        throw;
    }
    try {
        state.eval(parsed, v);
    } catch (Error & e) {
        e.addPrefix(format("While evaluating the output from ‘%1%’, at %2%\n") % program % pos);
        throw;
    }
}
