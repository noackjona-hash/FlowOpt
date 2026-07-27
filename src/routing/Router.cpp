#include "flowopt/routing/Router.hpp"

namespace flowopt {

// TODO: A*/Dijkstra ueber die CSR-Adjazenz implementieren.
// Grundgeruest-Stub: liefert vorerst keine Route.
std::uint32_t Router::route(NodeId /*from*/, NodeId /*to*/) {
    return kInvalidIdx;
}

std::span<const LaneId> Router::lanes(std::uint32_t handle) const {
    if (handle >= cache_.size()) {
        return {};
    }
    return cache_[handle];
}

void Router::clearCache() {
    cache_.clear();
}

} // namespace flowopt
