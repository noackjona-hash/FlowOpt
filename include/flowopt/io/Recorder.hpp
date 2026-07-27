#pragma once

#include "flowopt/sim/World.hpp"

#include <vector>

namespace flowopt {

// Zeichnet leichte Zustands-Snapshots auf (Fahrzeugpositionen je Step), damit
// ein Lauf spaeter ohne erneute Berechnung abgespielt oder gerendert werden kann.
class Recorder {
public:
    struct Frame {
        std::uint64_t stepIndex;
        Real          time;
        std::vector<LaneId> lane;
        std::vector<Real>   posOnLane;
    };

    void capture(const World& world);
    void clear() { frames_.clear(); }

    [[nodiscard]] const std::vector<Frame>& frames() const noexcept { return frames_; }

private:
    std::vector<Frame> frames_;
};

} // namespace flowopt
