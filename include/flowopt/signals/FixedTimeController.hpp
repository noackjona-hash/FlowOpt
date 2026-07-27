#pragma once

#include "flowopt/signals/ITrafficSignalController.hpp"

#include <vector>

namespace flowopt {

// Baseline-Ampel mit festem, zyklischem Phasenplan (feste Umlaufzeiten).
// Dient als Referenz, gegen die GA/RL-Controller gemessen werden.
class FixedTimeController final : public ITrafficSignalController {
public:
    void bind(NodeId intersection, std::span<const LaneId> approaches) override;
    void update(const SignalContext& ctx, Real dt, std::span<SignalState> out) override;
    void setParameters(std::span<const Real> params) override;
    [[nodiscard]] std::size_t parameterCount() const noexcept override;

private:
    NodeId intersection_{kInvalidNode};
    std::size_t laneCount_{0};

    // Parameter: Gruen- und Gelbdauer je Phase (hier vereinfacht global).
    Real greenDuration_{Real(20)};
    Real amberDuration_{Real(3)};

    Real phaseTimer_{Real(0)};
    std::size_t activePhase_{0};
    std::size_t phaseCount_{0};
};

} // namespace flowopt
