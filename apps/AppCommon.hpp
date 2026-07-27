#pragma once

#include "flowopt/io/ScenarioLoader.hpp"
#include "flowopt/signals/FixedTimeController.hpp"
#include "flowopt/signals/PolicyController.hpp"
#include "flowopt/signals/RuleBasedController.hpp"
#include "flowopt/sim/SimConfig.hpp"
#include "flowopt/sim/Simulation.hpp"

#include <charconv>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>

namespace flowopt {

// Minimaler CLI-Parser, gemeinsam von Headless- und GUI-App genutzt.
//   --steps N  --seed N  --scenario PATH  --controller fixed|rule|policy
//   --headless (default)  --gui  --steps-per-frame N
[[nodiscard]] inline SimConfig parseCli(int argc, char** argv) {
    SimConfig cfg;
    auto toU64 = [](std::string_view s, std::uint64_t def) {
        std::uint64_t v = def;
        std::from_chars(s.data(), s.data() + s.size(), v);
        return v;
    };

    for (int i = 1; i < argc; ++i) {
        std::string_view a = argv[i];
        auto next = [&]() -> std::string_view {
            return (i + 1 < argc) ? std::string_view{argv[++i]} : std::string_view{};
        };
        if (a == "--steps")           cfg.steps = toU64(next(), cfg.steps);
        else if (a == "--seed")       cfg.seed = toU64(next(), cfg.seed);
        else if (a == "--scenario")   cfg.scenarioPath = std::string(next());
        else if (a == "--steps-per-frame") cfg.stepsPerFrame = static_cast<std::uint32_t>(toU64(next(), 1));
        else if (a == "--headless")   cfg.gui = false;
        else if (a == "--gui")        cfg.gui = true;
        else if (a == "--controller") {
            std::string_view c = next();
            if (c == "rule")        cfg.controller = ControllerKind::RuleBased;
            else if (c == "policy") cfg.controller = ControllerKind::Policy;
            else                    cfg.controller = ControllerKind::FixedTime;
        }
    }
    return cfg;
}

[[nodiscard]] inline std::unique_ptr<ITrafficSignalController> makeController(ControllerKind kind) {
    switch (kind) {
        case ControllerKind::RuleBased: return std::make_unique<RuleBasedController>();
        case ControllerKind::Policy:    return std::make_unique<PolicyController>();
        case ControllerKind::FixedTime:
        default:                        return std::make_unique<FixedTimeController>();
    }
}

// Baut eine startklare Simulation aus der Config. Gemeinsamer Code beider Apps
// -> identische Physik in Headless und GUI.
[[nodiscard]] inline Simulation buildSimulation(const SimConfig& cfg) {
    World world = cfg.scenarioPath.empty() ? ScenarioLoader::demoGrid()
                                           : ScenarioLoader::fromFile(cfg.scenarioPath);
    Simulation sim(std::move(world), cfg);

    // Ein Controller je Kreuzung (Grundgeruest: global identischer Typ).
    for (std::size_t n = 0; n < sim.world().net.nodeCount(); ++n) {
        sim.addController(makeController(cfg.controller));
    }
    return sim;
}

} // namespace flowopt
