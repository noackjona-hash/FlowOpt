#pragma once

#include "flowopt/core/Types.hpp"
#include "flowopt/graph/RoadNetwork.hpp"

#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>

namespace flowopt {

// Berechnet Routen (Folge von Lanes) durch das Netz per A* und cached sie pro
// Start-Ziel-Paar (OD-Matrix). Eine Route ist die Lane-Folge, die ein Fahrzeug
// ab dem Startknoten bis zum Zielknoten abfaehrt.
class Router {
public:
    explicit Router(const RoadNetwork& net);

    // Liefert ein Handle in den internen Routen-Cache (A* nur beim ersten Mal).
    // Rueckgabe kInvalidIdx, falls kein Weg existiert.
    [[nodiscard]] std::uint32_t route(NodeId from, NodeId to);

    // Lane-Folge zu einem zuvor erzeugten Handle (leer bei ungueltigem Handle).
    [[nodiscard]] std::span<const LaneId> lanes(std::uint32_t handle) const;

    void clearCache();

    [[nodiscard]] std::size_t routeCount() const noexcept { return routes_.size(); }

private:
    // Fuehrt die eigentliche A*-Suche aus und legt die Route ab; gibt Handle zurueck.
    [[nodiscard]] std::uint32_t computeAStar(NodeId from, NodeId to);

    const RoadNetwork& net_;
    Real               invMaxSpeed_{1};  // 1 / hoechstes Tempolimit (fuer die Heuristik)

    std::vector<std::vector<LaneId>>                 routes_;   // handle -> Lane-Folge
    std::unordered_map<std::uint64_t, std::uint32_t> odCache_;  // (from,to) -> handle
};

} // namespace flowopt
