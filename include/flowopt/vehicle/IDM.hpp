#pragma once

#include "flowopt/core/Types.hpp"

#include <cmath>

namespace flowopt {

// Intelligent Driver Model (IDM) -- Laengsdynamik-Kernel.
// Reine, zustandslose Funktion: nimmt den eigenen Zustand plus die Luecke und
// Relativgeschwindigkeit zum Vordermann und liefert die Beschleunigung [m/s^2].
// Bewusst als freie, inline-Funktion gehalten, damit der Compiler sie im
// heissen Lane-Loop vollstaendig vektorisieren/inlinen kann.
//
//   gap    : Netto-Abstand zum Vordermann (Stossstange zu Stossstange) [m]
//   deltaV : eigene Geschwindigkeit minus Vordermann-Geschwindigkeit [m/s]
[[nodiscard]] inline Real idmAccel(Real v, Real v0, Real T, Real aMax, Real bComf,
                                   Real s0, Real gap, Real deltaV) noexcept {
    constexpr Real kDelta = Real(4);        // Beschleunigungsexponent
    constexpr Real kMinGap = Real(0.1);     // numerische Untergrenze fuer den Abstand

    const Real safeGap = gap > kMinGap ? gap : kMinGap;

    // Gewuenschter dynamischer Mindestabstand s*.
    const Real sStar = s0 + v * T + (v * deltaV) / (Real(2) * std::sqrt(aMax * bComf));
    const Real sStarClamped = sStar > Real(0) ? sStar : Real(0);

    const Real freeRoad = Real(1) - std::pow(v / v0, kDelta);
    const Real interaction = (sStarClamped / safeGap) * (sStarClamped / safeGap);

    return aMax * (freeRoad - interaction);
}

} // namespace flowopt
