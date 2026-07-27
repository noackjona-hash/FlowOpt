#pragma once

#include "flowopt/signals/ITrafficSignalController.hpp"

#include <vector>

namespace flowopt {

// Regel-/sensorbasierte Ampel (z.B. Max-Pressure oder "laengste Warteschlange
// zuerst"). Reagiert auf die Warteschlangenlaengen aus dem SignalContext.
class RuleBasedController final : public ITrafficSignalController {
public:
    void bind(NodeId intersection, std::span<const LaneId> approaches) override;
    void update(const SignalContext& ctx, Real dt, std::span<SignalState> out) override;
    void setParameters(std::span<const Real> params) override;
    [[nodiscard]] std::size_t parameterCount() const noexcept override;

private:
    NodeId intersection_{kInvalidNode};
    std::size_t laneCount_{0};

    Real minGreen_{Real(5)};   // Mindestgruenzeit gegen Flackern
    Real elapsed_{Real(0)};
    std::size_t servedLane_{0};
};

} // namespace flowopt
