#include "flowopt/signals/PolicyController.hpp"

namespace flowopt {

void PolicyController::bind(NodeId intersection, std::span<const LaneId> approaches) {
    intersection_ = intersection;
    laneCount_    = approaches.size();
    if (weights_.empty()) {
        weights_.assign(laneCount_, Real(1));
    }
}

// TODO: Policy-Evaluation (lineare Politik oder kleines MLP) implementieren.
// Grundgeruest-Stub: waehlt argmax(weight_i * queueLen_i) als Gruen-Lane.
void PolicyController::update(const SignalContext& ctx, Real /*dt*/, std::span<SignalState> out) {
    std::size_t best = 0;
    Real bestScore = -1;
    for (std::size_t i = 0; i < out.size(); ++i) {
        const Real q = (i < ctx.queueLengths.size())
                           ? static_cast<Real>(ctx.queueLengths[i]) : Real(0);
        const Real w = (i < weights_.size()) ? weights_[i] : Real(1);
        const Real score = w * q;
        if (score > bestScore) {
            bestScore = score;
            best = i;
        }
    }
    for (std::size_t i = 0; i < out.size(); ++i) {
        out[i] = (i == best) ? SignalState::Green : SignalState::Red;
    }
}

void PolicyController::setParameters(std::span<const Real> params) {
    weights_.assign(params.begin(), params.end());
}

std::size_t PolicyController::parameterCount() const noexcept {
    return weights_.size();
}

} // namespace flowopt
