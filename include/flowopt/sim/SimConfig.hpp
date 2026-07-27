#pragma once

#include "flowopt/core/Types.hpp"

#include <cstdint>
#include <string>

namespace flowopt {

// Wie die Ampeln gesteuert werden (waehlt die Controller-Implementierung).
enum class ControllerKind { FixedTime, RuleBased, Policy };

// Alle Laufzeitparameter eines Simulationslaufs an einem Ort (data-driven).
struct SimConfig {
    std::uint64_t steps    = 10'000;        // Anzahl Simulationsschritte (headless)
    std::uint64_t seed     = 42;            // RNG-Seed fuer Reproduzierbarkeit
    Real          dt       = kFixedDt;      // fester Zeitschritt
    Real          spawnRate = Real(0.2);    // erwartete Fahrzeuge pro Quelle und Sekunde

    ControllerKind controller = ControllerKind::FixedTime;

    std::string scenarioPath;               // leer => eingebautes Demo-Netz
    bool        gui           = false;      // true => GUI-App (nur mit FLOWOPT_GUI-Build)
    std::uint32_t stepsPerFrame = 1;        // GUI: Sim-Schritte pro gerendertem Frame
};

} // namespace flowopt
