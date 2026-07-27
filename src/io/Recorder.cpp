#include "flowopt/io/Recorder.hpp"

namespace flowopt {

void Recorder::capture(const World& world) {
    Frame frame;
    frame.stepIndex = world.stepIndex;
    frame.time      = world.time;
    frame.lane      = world.vehicles.lane;
    frame.posOnLane = world.vehicles.posOnLane;
    frames_.push_back(std::move(frame));
}

} // namespace flowopt
