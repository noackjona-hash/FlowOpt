#pragma once

#include "flowopt/render/IRenderer.hpp"

namespace flowopt {

// Konkrete Visualisierung mit Raylib. Wird NUR im GUI-Build (-DFLOWOPT_GUI=ON)
// kompiliert und gegen Raylib gelinkt; der Core weiss davon nichts.
class RaylibRenderer final : public IRenderer {
public:
    RaylibRenderer(int width = 1280, int height = 720, const char* title = "FlowOpt");
    ~RaylibRenderer() override;

    void draw(const World& world) override;
    [[nodiscard]] bool shouldClose() override;

private:
    Real pixelsPerMeter_{4};
};

} // namespace flowopt
