#include "flowopt/vehicle/VehiclePool.hpp"

#include <algorithm>

namespace flowopt {

VehicleId VehiclePool::spawn(LaneId startLane, NodeId dest, const Params& p) {
    std::uint32_t slot;
    if (!freeList.empty()) {
        slot = idx(freeList.back());
        freeList.pop_back();
    } else {
        slot = static_cast<std::uint32_t>(active.size());
        lane.emplace_back();
        posOnLane.emplace_back();
        speed.emplace_back();
        accel.emplace_back();
        v0.emplace_back();
        T.emplace_back();
        aMax.emplace_back();
        bComf.emplace_back();
        s0.emplace_back();
        length.emplace_back();
        destination.emplace_back();
        routeHandle.emplace_back();
        routeCursor.emplace_back();
        active.emplace_back();
    }

    lane[slot]        = startLane;
    posOnLane[slot]   = Real(0);
    speed[slot]       = Real(0);
    accel[slot]       = Real(0);
    v0[slot]          = p.v0;
    T[slot]           = p.T;
    aMax[slot]        = p.aMax;
    bComf[slot]       = p.bComf;
    s0[slot]          = p.s0;
    length[slot]      = p.length;
    destination[slot] = dest;
    routeHandle[slot] = kInvalidIdx;
    routeCursor[slot] = 0;
    active[slot]      = 1;

    return VehicleId{slot};
}

VehicleId VehiclePool::spawn(LaneId startLane, NodeId dest) {
    return spawn(startLane, dest, Params{});
}

void VehiclePool::despawn(VehicleId id) {
    const std::uint32_t slot = idx(id);
    if (slot >= active.size() || active[slot] == 0) {
        return;
    }
    active[slot] = 0;
    freeList.push_back(id);
}

std::size_t VehiclePool::activeCount() const noexcept {
    return static_cast<std::size_t>(std::count(active.begin(), active.end(), std::uint8_t{1}));
}

} // namespace flowopt
