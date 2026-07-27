#pragma once

#include "flowopt/signals/ITrafficSignalController.hpp"

#include <vector>

namespace flowopt {

// Adapter fuer KI-Steuerungen: interpretiert den Parametervektor als
// GA-Chromosom oder neuronale Gewichte. Genau hier docken GA/RL an, ohne dass
// die Engine oder die uebrigen Controller angefasst werden muessen.
class PolicyController final : public ITrafficSignalController {
public:
    void bind(NodeId intersection, std::span<const LaneId> approaches) override;
    void update(const SignalContext& ctx, Real dt, std::span<SignalState> out) override;
    void setParameters(std::span<const Real> params) override;
    [[nodiscard]] std::size_t parameterCount() const noexcept override;

private:
    NodeId intersection_{kInvalidNode};
    std::size_t laneCount_{0};

    // Flacher Gewichtsvektor der Policy (z.B. lineare Politik queueLen -> Score).
    std::vector<Real> weights_;
};

} // namespace flowopt
