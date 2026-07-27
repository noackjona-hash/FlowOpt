#pragma once

#include "flowopt/core/Types.hpp"
#include "flowopt/graph/RoadNetwork.hpp"
#include "flowopt/vehicle/VehiclePool.hpp"

#include <cstdint>
#include <span>

namespace flowopt {

// Zustand eines einzelnen Signals (einer Fahrspur-Freigabe).
enum class SignalState : std::uint8_t { Red, Green, Amber };

// Read-only Sicht auf die Welt, die einem Controller je Step gereicht wird.
// Der Controller darf den Zustand NICHT mutieren -- er liefert nur Freigaben zurueck.
struct SignalContext {
    const RoadNetwork& net;
    const VehiclePool& vehicles;
    Real simTime;
    // Sensorik pro gebundener Approach-Lane, z.B. Warteschlangenlaengen.
    std::span<const std::uint32_t> queueLengths;
};

// Abstrakte Schnittstelle fuer jede Ampelsteuerung.
// Statische, regelbasierte und KI-Controller implementieren dieselbe Schnittstelle
// und sind damit im Simulation-Loop austauschbar, ohne die Engine zu aendern.
class ITrafficSignalController {
public:
    virtual ~ITrafficSignalController() = default;

    // Einmalige Bindung an die zu steuernde Kreuzung und ihre Approach-Lanes.
    virtual void bind(NodeId intersection, std::span<const LaneId> approaches) = 0;

    // Pro Step aufgerufen. Schreibt fuer jede gebundene Lane den Zustand nach out.
    // out.size() == Anzahl der bei bind() uebergebenen Lanes.
    virtual void update(const SignalContext& ctx, Real dt, std::span<SignalState> out) = 0;

    // Fuer GA/RL: flacher Parametervektor (Phasenzeiten, Policy-Gewichte, ...).
    virtual void setParameters(std::span<const Real> params) = 0;
    [[nodiscard]] virtual std::size_t parameterCount() const noexcept = 0;
};

} // namespace flowopt
