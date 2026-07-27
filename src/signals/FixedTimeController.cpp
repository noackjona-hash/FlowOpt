#include "flowopt/signals/FixedTimeController.hpp"

#include <algorithm>

namespace flowopt {

void FixedTimeController::bind(NodeId intersection, std::span<const LaneId> approaches) {
    intersection_ = intersection;
    laneCount_    = approaches.size();
    // Vereinfachtes Modell: jede Approach-Lane ist eine eigene Phase.
    phaseCount_   = std::max<std::size_t>(1, laneCount_);
    activePhase_  = 0;
    phaseTimer_   = Real(0);
}

void FixedTimeController::update(const SignalContext& /*ctx*/, Real dt, std::span<SignalState> out) {
    phaseTimer_ += dt;
    const Real cycle = greenDuration_ + amberDuration_;
    if (phaseTimer_ >= cycle) {
        phaseTimer_ -= cycle;
        activePhase_ = (activePhase_ + 1) % phaseCount_;
    }

    const bool amber = phaseTimer_ >= greenDuration_;
    for (std::size_t i = 0; i < out.size(); ++i) {
        if (i == activePhase_) {
            out[i] = amber ? SignalState::Amber : SignalState::Green;
        } else {
            out[i] = SignalState::Red;
        }
    }
}

void FixedTimeController::setParameters(std::span<const Real> params) {
    if (params.size() >= 1) greenDuration_ = params[0];
    if (params.size() >= 2) amberDuration_ = params[1];
}

std::size_t FixedTimeController::parameterCount() const noexcept {
    return 2;
}

} // namespace flowopt
