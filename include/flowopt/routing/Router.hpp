#pragma once

#include "flowopt/core/Types.hpp"
#include "flowopt/graph/RoadNetwork.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace flowopt {

// Berechnet Routen (Folge von Lanes) durch das Netz und cached sie pro OD-Paar.
// Aktuell A*/Dijkstra geplant; hier nur die Schnittstelle des Grundgeruests.
class Router {
public:
    explicit Router(const RoadNetwork& net) : net_(net) {}

    // Berechnet die schnellste Route und liefert ein Handle in den internen Cache.
    // Rueckgabe kInvalidIdx, falls kein Weg existiert.
    [[nodiscard]] std::uint32_t route(NodeId from, NodeId to);

    // Liefert die Lane-Folge zu einem zuvor erzeugten Handle.
    [[nodiscard]] std::span<const LaneId> lanes(std::uint32_t handle) const;

    void clearCache();

private:
    const RoadNetwork& net_;
    std::vector<std::vector<LaneId>> cache_;
};

} // namespace flowopt
