#pragma once

#include "flowopt/core/Types.hpp"

#include <cstdint>

namespace flowopt {

// Deterministischer, schneller PRNG (xoshiro256**-Variante, header-only).
// Ein zentraler, geseedeter RNG ist Pflicht fuer reproduzierbare GA/RL-Laeufe.
class Rng {
public:
    explicit Rng(std::uint64_t seed = 0x9E3779B97F4A7C15ull) noexcept { reseed(seed); }

    void reseed(std::uint64_t seed) noexcept {
        // SplitMix64, um den Zustand aus einem einzelnen Seed zu fuellen.
        for (auto& s : state_) {
            seed += 0x9E3779B97F4A7C15ull;
            std::uint64_t z = seed;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
            s = z ^ (z >> 31);
        }
    }

    [[nodiscard]] std::uint64_t nextU64() noexcept {
        const std::uint64_t result = rotl(state_[1] * 5, 7) * 9;
        const std::uint64_t t = state_[1] << 17;
        state_[2] ^= state_[0];
        state_[3] ^= state_[1];
        state_[1] ^= state_[2];
        state_[0] ^= state_[3];
        state_[2] ^= t;
        state_[3] = rotl(state_[3], 45);
        return result;
    }

    // Gleichverteilt in [0, 1).
    [[nodiscard]] Real nextUnit() noexcept {
        return static_cast<Real>((nextU64() >> 40) * (1.0 / 16777216.0));
    }

    // Gleichverteilt in [lo, hi).
    [[nodiscard]] Real nextRange(Real lo, Real hi) noexcept {
        return lo + (hi - lo) * nextUnit();
    }

private:
    static std::uint64_t rotl(std::uint64_t x, int k) noexcept {
        return (x << k) | (x >> (64 - k));
    }

    std::uint64_t state_[4]{};
};

} // namespace flowopt
