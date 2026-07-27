#include "flowopt/sim/Simulation.hpp"

#include "flowopt/vehicle/IDM.hpp"

#include <utility>

namespace flowopt {

Simulation::Simulation(World world, SimConfig config)
    : world_(std::move(world)), config_(config), rng_(config.seed) {
    world_.syncSignalBuffer();
}

void Simulation::addController(std::unique_ptr<ITrafficSignalController> controller) {
    controllers_.push_back(std::move(controller));
}

void Simulation::step(Real dt) {
    phaseSpawn(dt);
    phaseSignals(dt);
    phaseVehicles(dt);
    phaseTransitions();
    phaseMetrics(dt);

    world_.time += dt;
    ++world_.stepIndex;
}

void Simulation::run(std::uint64_t nSteps) {
    for (std::uint64_t i = 0; i < nSteps; ++i) {
        step(config_.dt);
    }
    metrics_.stepsRun = world_.stepIndex;
}

// --- Phasen ----------------------------------------------------------------
// Grundgeruest: die Phasen sind als deterministische Reihenfolge angelegt.
// Die eigentliche Logik (Perception, Lane-Wechsel, Metrik-Erfassung) folgt
// in den naechsten Iterationen.

void Simulation::phaseSpawn(Real /*dt*/) {
    // TODO: Fahrzeuge aus OD-Nachfrage per rng_ in Quell-Lanes erzeugen.
}

void Simulation::phaseSignals(Real dt) {
    // TODO: pro Kreuzung SignalContext bauen und controller.update() aufrufen,
    // Ergebnis in world_.laneSignal schreiben.
    (void)dt;
}

void Simulation::phaseVehicles(Real dt) {
    // TODO: pro Lane den Vordermann bestimmen und IDM integrieren.
    // Skizze des heissen Kernels (semi-impliziter Euler):
    //   accel = idmAccel(v, v0, T, aMax, bComf, s0, gap, deltaV);
    //   v    += accel * dt;  if (v < 0) v = 0;
    //   posOnLane += v * dt;
    (void)dt;
}

void Simulation::phaseTransitions() {
    // TODO: Lane-Ende erreicht -> naechste Lane laut Route; Ziel erreicht -> despawn.
}

void Simulation::phaseMetrics(Real dt) {
    // TODO: Wartezeit/Stopps/Durchsatz akkumulieren.
    (void)dt;
}

} // namespace flowopt
