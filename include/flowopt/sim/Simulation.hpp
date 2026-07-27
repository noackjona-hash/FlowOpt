#pragma once

#include "flowopt/core/Rng.hpp"
#include "flowopt/core/Types.hpp"
#include "flowopt/io/ILogger.hpp"
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

    // Ein einzelner, deterministischer Simulationsschritt.
    void step(Real dt);

    // Headless-Batch: fuehrt n Schritte ohne jede Zeitmessung/Frame-Kopplung aus.
    void run(std::uint64_t nSteps);

    [[nodiscard]] const World&    world()   const noexcept { return world_; }
    [[nodiscard]] const Metrics&  metrics() const noexcept { return metrics_; }
    [[nodiscard]] const SimConfig& config() const noexcept { return config_; }

private:
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

    std::vector<std::unique_ptr<ITrafficSignalController>> controllers_;
};

} // namespace flowopt
