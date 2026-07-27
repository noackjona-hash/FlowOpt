#include "flowopt/routing/Router.hpp"

#include "flowopt/core/Math.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <utility>

namespace flowopt {

namespace {
// Packt ein OD-Paar in einen 64-Bit-Cache-Schluessel.
[[nodiscard]] std::uint64_t odKey(NodeId from, NodeId to) noexcept {
    return (static_cast<std::uint64_t>(idx(from)) << 32) | static_cast<std::uint64_t>(idx(to));
}
} // namespace

Router::Router(const RoadNetwork& net) : net_(net) {
    Real vmax = Real(1);
    for (const Real limit : net_.edgeSpeedLimit) {
        vmax = std::max(vmax, limit);
    }
    invMaxSpeed_ = Real(1) / vmax;
}

std::uint32_t Router::route(NodeId from, NodeId to) {
    const std::uint64_t key = odKey(from, to);
    if (const auto it = odCache_.find(key); it != odCache_.end()) {
        return it->second;  // Cache-Treffer -> keine erneute A*-Suche
    }
    const std::uint32_t handle = computeAStar(from, to);
    odCache_.emplace(key, handle);
    return handle;
}

std::span<const LaneId> Router::lanes(std::uint32_t handle) const {
    if (handle >= routes_.size()) {
        return {};
    }
    return routes_[handle];
}

void Router::clearCache() {
    routes_.clear();
    odCache_.clear();
}

std::uint32_t Router::computeAStar(NodeId from, NodeId to) {
    const std::size_t n = net_.nodeCount();
    if (idx(from) >= n || idx(to) >= n) {
        return kInvalidIdx;
    }

    // Bereits am Ziel -> leere (aber gueltige) Route.
    if (from == to) {
        const auto handle = static_cast<std::uint32_t>(routes_.size());
        routes_.emplace_back();
        return handle;
    }

    constexpr Real kInf = std::numeric_limits<Real>::max();
    std::vector<Real>         g(n, kInf);              // bekannte Kosten (Reisezeit) bis Knoten
    std::vector<EdgeId>       cameEdge(n, kInvalidEdge); // Kante, ueber die der Knoten erreicht wurde
    std::vector<std::uint8_t> closed(n, 0);

    // Zulaessige Heuristik: euklidische Distanz zum Ziel / hoechstes Tempolimit
    // (unterschaetzt die verbleibende Reisezeit -> A* bleibt optimal).
    const Vec2 goal = net_.nodePos[idx(to)];
    auto heuristic = [&](std::uint32_t node) -> Real {
        return distance(net_.nodePos[node], goal) * invMaxSpeed_;
    };

    using PQItem = std::pair<Real, std::uint32_t>;  // (f = g + h, node)
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<>> open;

    g[idx(from)] = Real(0);
    open.push({heuristic(idx(from)), idx(from)});

    while (!open.empty()) {
        const auto [f, u] = open.top();
        open.pop();
        if (closed[u]) {
            continue;  // veralteter Eintrag
        }
        if (u == idx(to)) {
            break;
        }
        closed[u] = 1;

        const std::uint32_t begin = net_.nodeOutStart[u];
        const std::uint32_t end   = net_.nodeOutStart[u + 1];
        for (std::uint32_t i = begin; i < end; ++i) {
            const std::uint32_t e = idx(net_.edgeOut[i]);
            const std::uint32_t v = idx(net_.edgeTo[e]);
            const Real cost = net_.edgeLength[e] / net_.edgeSpeedLimit[e];  // Reisezeit der Kante
            const Real tentative = g[u] + cost;
            if (tentative < g[v]) {
                g[v] = tentative;
                cameEdge[v] = EdgeId{e};
                open.push({tentative + heuristic(v), v});
            }
        }
    }

    if (g[idx(to)] == kInf) {
        return kInvalidIdx;  // kein Weg
    }

    // Kantenpfad rueckwaerts rekonstruieren und in Lanes uebersetzen.
    std::vector<LaneId> laneSeq;
    std::uint32_t cur = idx(to);
    while (cur != idx(from)) {
        const EdgeId e = cameEdge[cur];
        if (idx(e) == kInvalidIdx) {
            return kInvalidIdx;  // defekte Rekonstruktion (sollte nicht auftreten)
        }
        laneSeq.push_back(LaneId{net_.edgeLaneStart[idx(e)]});  // erste Lane der Kante
        cur = idx(net_.edgeFrom[idx(e)]);
    }
    std::reverse(laneSeq.begin(), laneSeq.end());

    const auto handle = static_cast<std::uint32_t>(routes_.size());
    routes_.push_back(std::move(laneSeq));
    return handle;
}

} // namespace flowopt
