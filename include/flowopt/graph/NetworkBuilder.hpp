#pragma once

#include "flowopt/core/Math.hpp"
#include "flowopt/core/Types.hpp"
#include "flowopt/graph/RoadNetwork.hpp"

#include <cstdint>

namespace flowopt {

// Baut ein RoadNetwork inkrementell auf und stellt am Ende die CSR-Strukturen her.
// Verwendung: addNode() / addEdge() / addLane() ... dann finalize().
class NetworkBuilder {
public:
    NodeId addNode(Vec2 position);

    // Fuegt eine gerichtete Kante mit gegebener Anzahl Lanes hinzu.
    EdgeId addEdge(NodeId from, NodeId to, Real length, Real speedLimit, std::uint8_t laneCount = 1);

    // Verbindet zwei Lanes (Standard-Nachfolger fuer Fahrzeuge ohne explizite Route).
    void connectLanes(LaneId from, LaneId to);

    // Ordnet einer Lane einen Signal-Slot (Ampel) zu.
    void attachSignal(LaneId lane, std::uint32_t signalIdx);

    // Baut CSR-Offsets, validiert die Topologie und gibt das fertige Netz zurueck.
    [[nodiscard]] RoadNetwork finalize();

private:
    RoadNetwork net_;
};

} // namespace flowopt
