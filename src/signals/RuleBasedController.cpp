#include "flowopt/signals/RuleBasedController.hpp"

namespace flowopt {

void RuleBasedController::bind(NodeId intersection, std::span<const LaneId> approaches) {
    intersection_ = intersection;
    laneCount_    = approaches.size();
    servedLane_   = 0;
    elapsed_      = Real(0);
}

void RuleBasedController::update(const SignalContext& ctx, Real dt, std::span<SignalState> out) {
    elapsed_ += dt;

    // Nach Ablauf der Mindestgruenzeit die Lane mit der laengsten Warteschlange waehlen.
    if (elapsed_ >= minGreen_ && !ctx.queueLengths.empty()) {
        std::size_t best = 0;
        std::uint32_t bestLen = 0;
        for (std::size_t i = 0; i < ctx.queueLengths.size(); ++i) {
            if (ctx.queueLengths[i] > bestLen) {
                bestLen = ctx.queueLengths[i];
                best = i;
            }
        }
        servedLane_ = best;
        elapsed_ = Real(0);
    }

    for (std::size_t i = 0; i < out.size(); ++i) {
        out[i] = (i == servedLane_) ? SignalState::Green : SignalState::Red;
    }
}

void RuleBasedController::setParameters(std::span<const Real> params) {
    if (params.size() >= 1) minGreen_ = params[0];
}

std::size_t RuleBasedController::parameterCount() const noexcept {
    return 1;
}

} // namespace flowopt
