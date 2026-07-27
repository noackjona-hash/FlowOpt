#pragma once

#include <cstdint>
#include <limits>

namespace flowopt {

// --- Strong ID types: verhindern das versehentliche Vertauschen von Indizes ---
enum class NodeId    : std::uint32_t {};
enum class EdgeId    : std::uint32_t {};
enum class LaneId    : std::uint32_t {};
enum class VehicleId : std::uint32_t {};

// Sentinel fuer "kein gueltiger Index".
inline constexpr std::uint32_t kInvalidIdx = std::numeric_limits<std::uint32_t>::max();

inline constexpr NodeId    kInvalidNode    { kInvalidIdx };
inline constexpr EdgeId    kInvalidEdge    { kInvalidIdx };
inline constexpr LaneId    kInvalidLane    { kInvalidIdx };
inline constexpr VehicleId kInvalidVehicle { kInvalidIdx };

// f32 reicht fuer die Fahrdynamik und verdoppelt den SIMD-Durchsatz gegenueber f64.
using Real = float;

// Fester Simulationszeitschritt (100 ms). Deterministisch und batch-freundlich.
inline constexpr Real kFixedDt = Real(0.1);

// Generischer Cast von einem starken ID-Typ auf seinen Roh-Index.
template <class E>
[[nodiscard]] constexpr std::uint32_t idx(E id) noexcept {
    return static_cast<std::uint32_t>(id);
}

} // namespace flowopt
