#include "AppCommon.hpp"

#include "flowopt/io/ILogger.hpp"

#include <chrono>
#include <cstdio>

// Headless-App: maximaler Durchsatz fuer GA/RL-Training. Keine Grafik, keine
// Echtzeit-Kopplung -- nur die reine Rechenschleife plus KPI-Ausgabe.
int main(int argc, char** argv) {
    using namespace flowopt;

    const SimConfig cfg = parseCli(argc, argv);
    Simulation sim = buildSimulation(cfg);

    NullLogger logger;
    sim.setLogger(&logger);

    const auto t0 = std::chrono::steady_clock::now();
    sim.run(cfg.steps);
    const auto t1 = std::chrono::steady_clock::now();

    const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    const Metrics& m = sim.metrics();

    std::printf("FlowOpt headless\n");
    std::printf("  scenario     : %s\n", cfg.scenarioPath.empty() ? "demoGrid" : cfg.scenarioPath.c_str());
    std::printf("  steps        : %llu\n", static_cast<unsigned long long>(m.stepsRun));
    std::printf("  nodes        : %zu\n", sim.world().net.nodeCount());
    std::printf("  lanes        : %zu\n", sim.world().net.laneCount());
    std::printf("  wall time    : %.2f ms\n", ms);
    std::printf("  steps/second : %.0f\n", m.stepsRun / (ms / 1000.0));
    std::printf("  fitness      : %.2f\n", m.fitness());
    return 0;
}
