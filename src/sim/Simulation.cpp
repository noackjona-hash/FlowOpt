#include "flowopt/sim/Simulation.hpp"

#include "flowopt/vehicle/IDM.hpp"

#include <algorithm>
#include <utility>

namespace flowopt {

namespace {
constexpr Real kMinGap    = Real(0.5);   // Mindest-Netto-Abstand [m]
constexpr Real kWaitSpeed = Real(0.5);   // unter dieser Geschwindigkeit gilt ein Fahrzeug als wartend
constexpr Real kSpawnGap  = Real(8.0);   // freier Platz an der Quelle, bevor neu erzeugt wird
constexpr Real kFreeRoad  = Real(1000);  // "unendliche" Luecke bei freier Fahrbahn
} // namespace

Simulation::Simulation(World world, SimConfig config)
    : world_(std::move(world)), config_(config), rng_(config.seed) {
    world_.syncSignalBuffer();

    const RoadNetwork& net = world_.net;

    // Zufahrts-Lanes je Kreuzung: alle Lanes, deren Kante an diesem Knoten endet.
    nodeApproaches_.assign(net.nodeCount(), {});
    for (std::size_t l = 0; l < net.laneCount(); ++l) {
        const EdgeId e = net.laneEdge[l];
        const NodeId to = net.edgeTo[idx(e)];
        nodeApproaches_[idx(to)].push_back(LaneId{static_cast<std::uint32_t>(l)});
    }

    // Entry-Lanes = Rand-Lanes ohne Vorgaenger (dienen als Fahrzeugquellen).
    std::vector<std::uint8_t> hasPred(net.laneCount(), 0);
    for (std::size_t l = 0; l < net.laneCount(); ++l) {
        const LaneId nxt = net.laneNextDefault[l];
        if (idx(nxt) != kInvalidIdx) {
            hasPred[idx(nxt)] = 1;
        }
    }
    for (std::size_t l = 0; l < net.laneCount(); ++l) {
        if (!hasPred[l]) {
            entryLanes_.push_back(LaneId{static_cast<std::uint32_t>(l)});
        }
    }
}

void Simulation::addController(std::unique_ptr<ITrafficSignalController> controller) {
    const std::size_t node = controllers_.size();
    if (node < nodeApproaches_.size()) {
        controller->bind(NodeId{static_cast<std::uint32_t>(node)}, nodeApproaches_[node]);
    }
    controllers_.push_back(std::move(controller));
}

void Simulation::step(Real dt) {
    rebuildOccupancy();
    phaseSpawn(dt);
    phaseSignals(dt);
    phaseVehicles(dt);
    phaseTransitions();
    phaseMetrics(dt);

    world_.time += dt;
    ++world_.stepIndex;
    metrics_.stepsRun = world_.stepIndex;
}

void Simulation::run(std::uint64_t nSteps) {
    for (std::uint64_t i = 0; i < nSteps; ++i) {
        step(config_.dt);
    }
}

// --- Belegung ---------------------------------------------------------------
// Baut die pro-Lane sortierten Fahrzeuglisten neu auf (aufsteigend nach Position,
// d.h. das letzte Element ist das vorderste Fahrzeug der Lane).
// TODO(perf): laneQueue spaeter inkrementell pflegen statt jeden Step zu sortieren.
void Simulation::rebuildOccupancy() {
    VehiclePool& v = world_.vehicles;
    for (auto& q : world_.net.laneQueue) {
        q.clear();
    }
    for (std::uint32_t s = 0; s < v.capacity(); ++s) {
        if (v.active[s]) {
            world_.net.laneQueue[idx(v.lane[s])].push_back(VehicleId{s});
        }
    }
    for (auto& q : world_.net.laneQueue) {
        std::sort(q.begin(), q.end(), [&](VehicleId a, VehicleId b) {
            return v.posOnLane[idx(a)] < v.posOnLane[idx(b)];
        });
    }
}

// --- Spawn ------------------------------------------------------------------
void Simulation::phaseSpawn(Real dt) {
    VehiclePool& v = world_.vehicles;
    const RoadNetwork& net = world_.net;
    const Real p = config_.spawnRate * dt;

    for (const LaneId entry : entryLanes_) {
        if (rng_.nextUnit() >= p) {
            continue;
        }
        // Nur erzeugen, wenn nahe der Quelle genug Platz ist.
        const auto& q = net.laneQueue[idx(entry)];
        if (!q.empty() && v.posOnLane[idx(q.front())] < kSpawnGap) {
            continue;
        }

        VehiclePool::Params params;
        const EdgeId e = net.laneEdge[idx(entry)];
        params.v0 = net.edgeSpeedLimit[idx(e)] * rng_.nextRange(Real(0.9), Real(1.1));

        const NodeId dest = net.edgeTo[idx(e)];
        const VehicleId id = v.spawn(entry, dest, params);
        ++metrics_.vehiclesSpawned;

        if (idx(id) < wasStopped_.size()) {
            wasStopped_[idx(id)] = 0;
        }
    }
}

// --- Signale ----------------------------------------------------------------
void Simulation::phaseSignals(Real dt) {
    const RoadNetwork& net = world_.net;

    for (std::size_t n = 0; n < controllers_.size() && n < nodeApproaches_.size(); ++n) {
        const auto& approaches = nodeApproaches_[n];
        if (approaches.empty()) {
            continue;
        }

        queueScratch_.assign(approaches.size(), 0);
        stateScratch_.assign(approaches.size(), SignalState::Red);
        for (std::size_t i = 0; i < approaches.size(); ++i) {
            queueScratch_[i] = static_cast<std::uint32_t>(net.laneQueue[idx(approaches[i])].size());
        }

        const SignalContext ctx{net, world_.vehicles, world_.time, queueScratch_};
        controllers_[n]->update(ctx, dt, stateScratch_);

        for (std::size_t i = 0; i < approaches.size(); ++i) {
            world_.laneSignal[idx(approaches[i])] = stateScratch_[i];
        }
    }
}

// --- Fahrzeugdynamik (IDM) --------------------------------------------------
// Zwei Durchlaeufe fuer eine ordnungsunabhaengige, deterministische Aktualisierung:
//   Pass 1 berechnet alle Beschleunigungen aus dem aktuellen Zustand,
//   Pass 2 integriert Geschwindigkeit/Position (semi-impliziter Euler).
void Simulation::phaseVehicles(Real dt) {
    VehiclePool& v = world_.vehicles;
    const RoadNetwork& net = world_.net;

    accelScratch_.assign(v.capacity(), Real(0));

    // Pass 1: Beschleunigungen.
    for (std::size_t l = 0; l < net.laneCount(); ++l) {
        const auto& q = net.laneQueue[l];
        const Real laneLen = net.laneLength[l];

        for (std::size_t k = 0; k < q.size(); ++k) {
            const std::uint32_t s = idx(q[k]);
            Real gap;
            Real deltaV;

            if (k + 1 < q.size()) {
                // Vordermann auf derselben Lane.
                const std::uint32_t lead = idx(q[k + 1]);
                gap = (v.posOnLane[lead] - v.length[lead]) - v.posOnLane[s];
                deltaV = v.speed[s] - v.speed[lead];
            } else {
                // Vorderstes Fahrzeug: Ampel bzw. naechste Lane betrachten.
                const Real distToEnd = laneLen - v.posOnLane[s];
                const SignalState sig = world_.laneSignal[l];

                if (sig != SignalState::Green) {
                    // Rot/Gelb wirkt wie ein stehendes Hindernis an der Haltelinie.
                    gap = distToEnd;
                    deltaV = v.speed[s];
                } else {
                    const LaneId nextLane = net.laneNextDefault[l];
                    const auto* nq = (idx(nextLane) != kInvalidIdx)
                                         ? &net.laneQueue[idx(nextLane)] : nullptr;
                    if (nq && !nq->empty()) {
                        const std::uint32_t lead = idx(nq->front());
                        gap = distToEnd + (v.posOnLane[lead] - v.length[lead]);
                        deltaV = v.speed[s] - v.speed[lead];
                    } else {
                        gap = kFreeRoad; // freie Fahrbahn
                        deltaV = Real(0);
                    }
                }
            }

            accelScratch_[s] = idmAccel(v.speed[s], v.v0[s], v.T[s], v.aMax[s],
                                        v.bComf[s], v.s0[s], gap, deltaV);
        }
    }

    // Pass 2: Integration mit Kollisions- und Haltelinien-Clamp.
    for (std::size_t l = 0; l < net.laneCount(); ++l) {
        const auto& q = net.laneQueue[l];
        const Real laneLen = net.laneLength[l];

        for (std::size_t k = 0; k < q.size(); ++k) {
            const std::uint32_t s = idx(q[k]);

            Real newSpeed = v.speed[s] + accelScratch_[s] * dt;
            if (newSpeed < Real(0)) newSpeed = Real(0);

            Real newPos = v.posOnLane[s] + newSpeed * dt;

            // Nicht in den Vordermann fahren (Positionen vor der Integration).
            if (k + 1 < q.size()) {
                const std::uint32_t lead = idx(q[k + 1]);
                const Real upper = v.posOnLane[lead] - v.length[lead] - kMinGap;
                newPos = std::min(newPos, upper);
            } else if (world_.laneSignal[l] != SignalState::Green) {
                // Vorderstes Fahrzeug haelt an roter/gelber Ampel vor der Haltelinie.
                newPos = std::min(newPos, laneLen);
            }

            // Niemals rueckwaerts rollen.
            if (newPos < v.posOnLane[s]) newPos = v.posOnLane[s];

            v.speed[s] = newSpeed;
            v.posOnLane[s] = newPos;
            v.accel[s] = accelScratch_[s];
        }
    }
}

// --- Uebergaenge ------------------------------------------------------------
void Simulation::phaseTransitions() {
    VehiclePool& v = world_.vehicles;
    const RoadNetwork& net = world_.net;

    for (std::uint32_t s = 0; s < v.capacity(); ++s) {
        if (!v.active[s]) {
            continue;
        }
        const std::uint32_t l = idx(v.lane[s]);
        if (v.posOnLane[s] < net.laneLength[l]) {
            continue;
        }

        const LaneId nextLane = net.laneNextDefault[l];
        if (idx(nextLane) == kInvalidIdx) {
            // Netzgrenze erreicht -> Fahrzeug verlaesst die Simulation.
            v.despawn(VehicleId{s});
            ++metrics_.vehiclesArrived;
        } else {
            // In die naechste Lane uebertreten und den Ueberlauf mitnehmen.
            const Real overflow = v.posOnLane[s] - net.laneLength[l];
            v.lane[s] = nextLane;
            v.posOnLane[s] = std::min(overflow, net.laneLength[idx(nextLane)]);
        }
    }
}

// --- Metriken ---------------------------------------------------------------
void Simulation::phaseMetrics(Real dt) {
    VehiclePool& v = world_.vehicles;
    if (wasStopped_.size() < v.capacity()) {
        wasStopped_.resize(v.capacity(), 0);
    }

    for (std::uint32_t s = 0; s < v.capacity(); ++s) {
        if (!v.active[s]) {
            continue;
        }
        const Real spd = v.speed[s];
        metrics_.totalTravelTime += dt;
        metrics_.distanceTravelled += static_cast<double>(spd) * dt;

        const bool stopped = spd < kWaitSpeed;
        if (stopped) {
            metrics_.totalWaitTime += dt;
            if (!wasStopped_[s]) {
                ++metrics_.totalStops;
            }
        }
        wasStopped_[s] = stopped ? 1 : 0;
    }
}

} // namespace flowopt
