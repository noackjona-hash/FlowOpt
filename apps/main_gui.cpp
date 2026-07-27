#include "AppCommon.hpp"

#include "flowopt/io/ILogger.hpp"
#include "flowopt/render/RaylibRenderer.hpp"

// GUI-App: nur mit -DFLOWOPT_GUI=ON gebaut. Die Sim-Physik ist identisch zur
// Headless-Variante; hier koppeln wir zusaetzlich an Frames und rendern lesend.
int main(int argc, char** argv) {
    using namespace flowopt;

    SimConfig cfg = parseCli(argc, argv);
    cfg.gui = true;
    Simulation sim = buildSimulation(cfg);

    ConsoleLogger logger;
    sim.setLogger(&logger);

    RaylibRenderer renderer;
    while (!renderer.shouldClose()) {
        for (std::uint32_t i = 0; i < cfg.stepsPerFrame; ++i) {
            sim.step(cfg.dt);
        }
        renderer.draw(sim.world());
    }
    return 0;
}
