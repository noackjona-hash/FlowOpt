#pragma once

#include "flowopt/core/Types.hpp"

#include <algorithm>
#include <cmath>

namespace flowopt {

// Leichtgewichtiger 2D-Vektor fuer Weltkoordinaten (Geometrie, Rendering).
struct Vec2 {
    Real x{0};
    Real y{0};

    [[nodiscard]] constexpr Vec2 operator+(Vec2 o) const noexcept { return {x + o.x, y + o.y}; }
    [[nodiscard]] constexpr Vec2 operator-(Vec2 o) const noexcept { return {x - o.x, y - o.y}; }
    [[nodiscard]] constexpr Vec2 operator*(Real s) const noexcept { return {x * s, y * s}; }
};

[[nodiscard]] inline Real length(Vec2 v) noexcept {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

[[nodiscard]] inline Real distance(Vec2 a, Vec2 b) noexcept {
    return length(b - a);
}

// Lineare Interpolation zwischen a und b.
[[nodiscard]] constexpr Vec2 lerp(Vec2 a, Vec2 b, Real t) noexcept {
    return a + (b - a) * t;
}

// Schnelles Clamp fuer Skalare (haeufig im IDM-Kernel).
[[nodiscard]] constexpr Real clampReal(Real v, Real lo, Real hi) noexcept {
    return std::min(std::max(v, lo), hi);
}

} // namespace flowopt
