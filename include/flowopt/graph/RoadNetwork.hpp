#pragma once

#include "flowopt/core/Math.hpp"
#include "flowopt/core/Types.hpp"

#include <cstdint>
#include <vector>

namespace flowopt {

// Straszennetz als gerichteter Graph in Structure-of-Arrays (SoA)-Layout.
//   Knoten = Kreuzungen, Kanten = gerichtete Strassen, Lanes = einzelne Fahrspuren.
// Fahrzeuge fahren auf Lanes; der IDM-Kernel iteriert lane-weise ueber dichte
// float-Arrays (cache-freundlich, auto-vektorisierbar). Nach dem Aufbau read-mostly.
struct RoadNetwork {
    // --- Knoten (Kreuzungen) ---
    std::vector<Vec2>          nodePos;        // Weltkoordinate
    std::vector<std::uint32_t> nodeOutStart;   // CSR-Offset in edgeOut (Grad + 1 Eintraege)
    std::vector<EdgeId>        edgeOut;         // CSR-Adjazenz: ausgehende Kanten je Knoten

    // --- Kanten (gerichtete Strassen) ---
    std::vector<NodeId>        edgeFrom;
    std::vector<NodeId>        edgeTo;
    std::vector<Real>          edgeLength;      // Meter
    std::vector<Real>          edgeSpeedLimit;  // m/s
    std::vector<std::uint32_t> edgeLaneStart;   // CSR-Offset in laneEdge
    std::vector<std::uint8_t>  edgeLaneCount;

    // --- Lanes (Fahrspuren; feinste Simulationseinheit) ---
    std::vector<EdgeId>        laneEdge;        // Zugehoerige Kante
    std::vector<Real>          laneLength;      // Meter
    std::vector<LaneId>        laneNextDefault; // Standard-Nachfolge-Lane (Routing kann ueberschreiben)
    std::vector<std::uint32_t> laneSignalIdx;   // Index in die Signaltabelle oder kInvalidIdx

    // --- Belegung: pro Lane die Fahrzeuge, geordnet nach Laengsposition ---
    // Ermoeglicht O(1)-Zugriff auf den Vordermann waehrend der IDM-Phase.
    std::vector<std::vector<VehicleId>> laneQueue;

    [[nodiscard]] std::size_t nodeCount() const noexcept { return nodePos.size(); }
    [[nodiscard]] std::size_t edgeCount() const noexcept { return edgeFrom.size(); }
    [[nodiscard]] std::size_t laneCount() const noexcept { return laneEdge.size(); }
};

} // namespace flowopt
