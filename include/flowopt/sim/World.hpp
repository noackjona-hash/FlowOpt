#pragma once

#include "flowopt/core/Types.hpp"
#include "flowopt/graph/RoadNetwork.hpp"
#include "flowopt/signals/ITrafficSignalController.hpp"
#include "flowopt/vehicle/VehiclePool.hpp"

#include <cstdint>
#include <vector>

namespace flowopt {

// Vollstaendiger Simulationszustand an einem Ort. Trivial serialisier-/kopierbar
// -> Snapshots fuer Replay, Checkpointing (RL) oder Reset zwischen GA-Generationen.
struct World {
    RoadNetwork              net;         // statisch nach dem Aufbau
    VehiclePool              vehicles;    // dynamisch
    std::vector<SignalState> laneSignal;  // aktueller Ampelzustand je Lane

    Real          time      = Real(0);
    std::uint64_t stepIndex = 0;

    // Bringt laneSignal auf die Lane-Anzahl des Netzes. Default Gruen: nur
    // kontrollierte Zufahrts-Lanes werden von einem Controller auf Rot gesetzt.
    void syncSignalBuffer() {
        laneSignal.assign(net.laneCount(), SignalState::Green);
    }
};

} // namespace flowopt
