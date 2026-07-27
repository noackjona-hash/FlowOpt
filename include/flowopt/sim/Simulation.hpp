#pragma once

#include "flowopt/core/Rng.hpp"
#include "flowopt/core/Types.hpp"
#include "flowopt/io/ILogger.hpp"
#include "flowopt/routing/Router.hpp"
#include "flowopt/signals/ITrafficSignalController.hpp"
#include "flowopt/sim/Metrics.hpp"
#include "flowopt/sim/SimConfig.hpp"
#include "flowopt/sim/World.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace flowopt {

// Kern der Simulation: haelt den gesamten Zustand und treibt den festen
// Zeitschritt-Loop. Kennt kein Rendering und keine Echtzeit -- reine Berechnung.
class Simulation {
public:
    Simulation(World world, SimConfig config);

    // Registriert einen Controller (Ownership geht ueber). Reihenfolge = Kreuzungsindex.
    void addController(std::unique_ptr<ITrafficSignalController> controller);

    void setLogger(ILogger* logger) noexcept { logger_ = logger; }

    // --- Controller-Parameter (fuer GA/RL) ---
    // Gesamtzahl der Parameter ueber alle Controller (flacher Optimierungsvektor).
    [[nodiscard]] std::size_t parameterCount() const noexcept;
    // Verteilt einen flachen Parametervektor der Reihe nach auf alle Controller.
    void setParameters(std::span<const Real> flat);

    // Ein einzelner, deterministischer Simulationsschritt.
    void step(Real dt);

    // Headless-Batch: fuehrt n Schritte ohne jede Zeitmessung/Frame-Kopplung aus.
    void run(std::uint64_t nSteps);

    [[nodiscard]] const World&    world()   const noexcept { return world_; }
    [[nodiscard]] const Metrics&  metrics() const noexcept { return metrics_; }
    [[nodiscard]] const SimConfig& config() const noexcept { return config_; }

private:
    // Sortiert laneQueue je Lane nach Laengsposition (Vordermann-Lookup in O(1)).
    void rebuildOccupancy();

    // Naechste Lane eines Fahrzeugs: folgt seiner A*-Route, faellt sonst auf
    // die Standard-Spurfolge (laneNextDefault) zurueck. kInvalidLane => Routenende.
    [[nodiscard]] LaneId nextLaneForVehicle(std::uint32_t slot) const;

    // Die einzelnen, deterministisch geordneten Phasen eines Steps.
    void phaseSpawn(Real dt);
    void phaseSignals(Real dt);
    void phaseVehicles(Real dt);
    void phaseTransitions();
    void phaseMetrics(Real dt);

    World      world_;
    SimConfig  config_;
    Metrics    metrics_;
    Rng        rng_;
    ILogger*   logger_{nullptr};
    Router     router_;

    std::vector<std::unique_ptr<ITrafficSignalController>> controllers_;

    // --- Topologie-Vorberechnung (einmalig im Konstruktor) ---
    std::vector<std::vector<LaneId>> nodeApproaches_; // Zufahrts-Lanes je Kreuzung
    std::vector<LaneId>              entryLanes_;      // Rand-Lanes ohne Vorgaenger (Quellen)

    // --- Wiederverwendete Scratch-Buffer (vermeiden Allokation pro Step) ---
    std::vector<Real>          accelScratch_;  // je Fahrzeug-Slot
    std::vector<std::uint32_t> queueScratch_;  // je Approach einer Kreuzung
    std::vector<SignalState>   stateScratch_;
    std::vector<std::uint8_t>  wasStopped_;    // fuer Stop-Zaehlung je Slot
};

} // namespace flowopt
