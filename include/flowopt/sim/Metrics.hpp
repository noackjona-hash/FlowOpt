#pragma once

#include "flowopt/core/Types.hpp"

#include <cstdint>

namespace flowopt {

// Aggregierte Kennzahlen eines Laufs. Bilden die Grundlage der Fitness-Funktion
// fuer GA/RL (z.B. minimiere Gesamtwartezeit / maximiere Durchsatz).
struct Metrics {
    std::uint64_t stepsRun        = 0;
    std::uint64_t vehiclesSpawned = 0;
    std::uint64_t vehiclesArrived = 0;
    std::uint64_t totalStops      = 0;

    double totalWaitTime     = 0.0;   // aufsummierte Wartezeit aller Fahrzeuge [s]
    double totalTravelTime   = 0.0;   // aufsummierte Reisezeit [s]
    double distanceTravelled = 0.0;   // gesamte gefahrene Strecke [m]

    // Beispiel-Fitness: kleiner ist besser (Wartezeit dominiert).
    [[nodiscard]] double fitness() const noexcept {
        const double throughput = static_cast<double>(vehiclesArrived);
        return totalWaitTime - 10.0 * throughput;
    }

    void reset() { *this = Metrics{}; }
};

} // namespace flowopt
