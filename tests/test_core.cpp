// Leichte, framework-freie Smoke-Tests fuer das Grundgeruest.
// Nutzt <cassert>, damit noch keine Test-Dependency gebraucht wird.
#include "flowopt/io/ScenarioLoader.hpp"
#include "flowopt/signals/FixedTimeController.hpp"
#include "flowopt/sim/SimConfig.hpp"
#include "flowopt/sim/Simulation.hpp"
#include "flowopt/vehicle/IDM.hpp"

#include <cassert>
#include <cstdio>
#include <memory>

using namespace flowopt;

static void testDemoGrid() {
    World w = ScenarioLoader::demoGrid();
    assert(w.net.nodeCount() == 4);
    assert(w.net.edgeCount() == 6);   // 3 Segmente * 2 Richtungen
    assert(w.net.laneCount() == 6);
    assert(w.laneSignal.size() == w.net.laneCount());
}

static void testVehiclePool() {
    VehiclePool pool;
    const VehicleId a = pool.spawn(LaneId{0}, NodeId{3});
    const VehicleId b = pool.spawn(LaneId{1}, NodeId{3});
    assert(pool.activeCount() == 2);
    pool.despawn(a);
    assert(pool.activeCount() == 1);
    const VehicleId c = pool.spawn(LaneId{2}, NodeId{3});
    assert(idx(c) == idx(a));          // Slot wurde per Free-List wiederverwendet
    assert(pool.isActive(b));
}

static void testIdmDecelerates() {
    // Nahe am langsamen Vordermann muss die Beschleunigung negativ sein.
    const Real acc = idmAccel(/*v*/ 13.0f, /*v0*/ 13.9f, /*T*/ 1.5f, /*aMax*/ 1.2f,
                              /*bComf*/ 2.0f, /*s0*/ 2.0f, /*gap*/ 3.0f, /*deltaV*/ 8.0f);
    assert(acc < 0.0f);
}

static void testSimulationRuns() {
    SimConfig cfg;
    cfg.steps = 5000;
    cfg.spawnRate = Real(0.5);
    Simulation sim(ScenarioLoader::demoGrid(), cfg);
    for (std::size_t n = 0; n < sim.world().net.nodeCount(); ++n) {
        sim.addController(std::make_unique<FixedTimeController>());
    }
    sim.run(cfg.steps);

    const Metrics& m = sim.metrics();
    assert(sim.world().stepIndex == 5000);
    assert(m.stepsRun == 5000);
    assert(m.vehiclesSpawned > 0);          // Quellen erzeugen Fahrzeuge
    assert(m.vehiclesArrived > 0);          // Fahrzeuge erreichen den Netzrand
    assert(m.distanceTravelled > 0.0);      // Fahrzeuge bewegen sich
    assert(m.vehiclesArrived <= m.vehiclesSpawned);
}

static void testDeterminism() {
    auto runOnce = [] {
        SimConfig cfg;
        cfg.steps = 3000;
        cfg.seed = 123;
        Simulation sim(ScenarioLoader::demoGrid(), cfg);
        for (std::size_t n = 0; n < sim.world().net.nodeCount(); ++n) {
            sim.addController(std::make_unique<FixedTimeController>());
        }
        sim.run(cfg.steps);
        return sim.metrics().vehiclesArrived;
    };
    assert(runOnce() == runOnce());          // gleicher Seed -> identisches Ergebnis
}

int main() {
    testDemoGrid();
    testVehiclePool();
    testIdmDecelerates();
    testSimulationRuns();
    testDeterminism();
    std::printf("all tests passed\n");
    return 0;
}
