#include "flowopt/io/ScenarioLoader.hpp"

#include "flowopt/graph/NetworkBuilder.hpp"

namespace flowopt {

// TODO: JSON/CSV-Parser anbinden. Grundgeruest-Stub: liefert das Demo-Netz.
World ScenarioLoader::fromFile(const std::string& /*path*/) {
    return demoGrid();
}

World ScenarioLoader::demoGrid() {
    NetworkBuilder builder;

    // Minimaler Korridor: 4 Kreuzungen in einer Reihe, je 100 m Abstand.
    constexpr int kNodes = 4;
    NodeId nodes[kNodes];
    for (int i = 0; i < kNodes; ++i) {
        nodes[i] = builder.addNode(Vec2{Real(i) * Real(100), Real(0)});
    }

    // Verbindungen in beide Richtungen (Einbahn-Lanes je Kante).
    constexpr Real kLimit = Real(13.9); // ~50 km/h
    for (int i = 0; i + 1 < kNodes; ++i) {
        builder.addEdge(nodes[i], nodes[i + 1], Real(100), kLimit, 1);
        builder.addEdge(nodes[i + 1], nodes[i], Real(100), kLimit, 1);
    }

    // Lane-Indizes entsprechen der Reihenfolge oben:
    //   vorwaerts (0->3): lane0 -> lane2 -> lane4
    //   rueckwaerts(3->0): lane5 -> lane3 -> lane1
    builder.connectLanes(LaneId{0}, LaneId{2});
    builder.connectLanes(LaneId{2}, LaneId{4});
    builder.connectLanes(LaneId{5}, LaneId{3});
    builder.connectLanes(LaneId{3}, LaneId{1});

    World world;
    world.net = builder.finalize();
    world.syncSignalBuffer();
    return world;
}

} // namespace flowopt
