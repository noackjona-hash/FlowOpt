#pragma once

#include "flowopt/core/Types.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace flowopt {

// Zentraler Fahrzeugspeicher als Structure-of-Arrays (SoA).
// Fahrzeuge werden nie einzeln allokiert; VehicleId indiziert alle Arrays.
// Freie Slots werden ueber eine Free-List wiederverwendet -> Spawn/Despawn
// ohne Reallokation und ohne Fragmentierung.
struct VehiclePool {
    // --- Dynamischer Zustand (jeden Step geschrieben) ---
    std::vector<LaneId> lane;       // aktuelle Fahrspur
    std::vector<Real>   posOnLane;  // Laengsposition auf der Lane [m]
    std::vector<Real>   speed;      // m/s
    std::vector<Real>   accel;      // m/s^2 (IDM-Ergebnis, fuer Debug/Rendering)

    // --- IDM-Parameter (quasi-statisch, pro Fahrzeug variierbar) ---
    std::vector<Real>   v0;         // Wunschgeschwindigkeit [m/s]
    std::vector<Real>   T;          // Sicherheits-Zeitluecke [s]
    std::vector<Real>   aMax;       // maximale Beschleunigung [m/s^2]
    std::vector<Real>   bComf;      // komfortable Verzoegerung [m/s^2]
    std::vector<Real>   s0;         // Mindestabstand im Stau [m]
    std::vector<Real>   length;     // Fahrzeuglaenge [m]

    // --- Routing ---
    std::vector<NodeId>        destination;
    std::vector<std::uint32_t> routeHandle;  // Index in den Router-Routen-Cache
    std::vector<std::uint32_t> routeCursor;  // aktuelle Position innerhalb der Route

    // --- Verwaltung ---
    std::vector<std::uint8_t> active;    // Belegt-Flag je Slot
    std::vector<VehicleId>    freeList;  // wiederverwendbare Slots

    // IDM-Standardwerte fuer neu erzeugte Fahrzeuge.
    struct Params {
        Real v0     = Real(13.9); // ~50 km/h
        Real T      = Real(1.5);
        Real aMax   = Real(1.2);
        Real bComf  = Real(2.0);
        Real s0     = Real(2.0);
        Real length = Real(4.5);
    };

    // Erzeugt ein Fahrzeug (nutzt Free-List wenn moeglich) und gibt seine Id zurueck.
    [[nodiscard]] VehicleId spawn(LaneId startLane, NodeId dest, const Params& p = Params{});

    // Gibt einen Slot frei (Fahrzeug hat sein Ziel erreicht).
    void despawn(VehicleId id);

    [[nodiscard]] std::size_t capacity() const noexcept { return active.size(); }
    [[nodiscard]] std::size_t activeCount() const noexcept;
    [[nodiscard]] bool isActive(VehicleId id) const noexcept { return active[idx(id)] != 0; }
};

} // namespace flowopt
