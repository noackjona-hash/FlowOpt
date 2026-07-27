#include "flowopt/graph/NetworkBuilder.hpp"

#include <algorithm>
#include <cassert>

namespace flowopt {

NodeId NetworkBuilder::addNode(Vec2 position) {
    const auto id = static_cast<std::uint32_t>(net_.nodePos.size());
    net_.nodePos.push_back(position);
    return NodeId{id};
}

EdgeId NetworkBuilder::addEdge(NodeId from, NodeId to, Real length, Real speedLimit,
                               std::uint8_t laneCount) {
    const auto edgeId = static_cast<std::uint32_t>(net_.edgeFrom.size());
    net_.edgeFrom.push_back(from);
    net_.edgeTo.push_back(to);
    net_.edgeLength.push_back(length);
    net_.edgeSpeedLimit.push_back(speedLimit);
    net_.edgeLaneStart.push_back(static_cast<std::uint32_t>(net_.laneEdge.size()));
    net_.edgeLaneCount.push_back(laneCount);

    for (std::uint8_t i = 0; i < laneCount; ++i) {
        net_.laneEdge.push_back(EdgeId{edgeId});
        net_.laneLength.push_back(length);
        net_.laneNextDefault.push_back(kInvalidLane);
        net_.laneSignalIdx.push_back(kInvalidIdx);
    }
    return EdgeId{edgeId};
}

void NetworkBuilder::connectLanes(LaneId from, LaneId to) {
    assert(idx(from) < net_.laneNextDefault.size());
    net_.laneNextDefault[idx(from)] = to;
}

void NetworkBuilder::attachSignal(LaneId lane, std::uint32_t signalIdx) {
    assert(idx(lane) < net_.laneSignalIdx.size());
    net_.laneSignalIdx[idx(lane)] = signalIdx;
}

RoadNetwork NetworkBuilder::finalize() {
    const std::size_t nNodes = net_.nodePos.size();
    const std::size_t nEdges = net_.edgeFrom.size();

    // CSR-Adjazenz der ausgehenden Kanten je Knoten aufbauen.
    std::vector<std::uint32_t> outDegree(nNodes, 0);
    for (std::size_t e = 0; e < nEdges; ++e) {
        ++outDegree[idx(net_.edgeFrom[e])];
    }

    net_.nodeOutStart.assign(nNodes + 1, 0);
    for (std::size_t n = 0; n < nNodes; ++n) {
        net_.nodeOutStart[n + 1] = net_.nodeOutStart[n] + outDegree[n];
    }

    net_.edgeOut.assign(nEdges, kInvalidEdge);
    std::vector<std::uint32_t> cursor(net_.nodeOutStart.begin(), net_.nodeOutStart.end());
    for (std::size_t e = 0; e < nEdges; ++e) {
        const std::uint32_t n = idx(net_.edgeFrom[e]);
        net_.edgeOut[cursor[n]++] = EdgeId{static_cast<std::uint32_t>(e)};
    }

    // Belegungsspeicher pro Lane vorbereiten.
    net_.laneQueue.assign(net_.laneEdge.size(), {});

    return std::move(net_);
}

} // namespace flowopt
