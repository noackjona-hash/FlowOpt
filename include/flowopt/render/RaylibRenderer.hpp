#pragma once

#include "flowopt/core/Math.hpp"
#include "flowopt/render/IRenderer.hpp"

namespace flowopt {

// Konkrete Visualisierung mit Raylib. Wird NUR im GUI-Build (-DFLOWOPT_GUI=ON)
// kompiliert und gegen Raylib gelinkt; der Core weiss davon nichts. Der Renderer
// liest den World-Zustand ausschliesslich lesend (frame-unabhaengig zur Physik).
class RaylibRenderer final : public IRenderer {
public:
    RaylibRenderer(int width = 1280, int height = 720, const char* title = "FlowOpt");
    ~RaylibRenderer() override;

    void draw(const World& world) override;
    [[nodiscard]] bool shouldClose() override;

private:
    // Passt die Welt-nach-Bildschirm-Transformation einmalig an die Netz-Ausdehnung an.
    void fitToNetwork(const World& world);
    [[nodiscard]] Vec2 worldToScreen(Vec2 p) const;

    int  width_;
    int  height_;
    bool transformReady_{false};

    Real scale_{1};   // Pixel pro Meter
    Vec2 worldMin_{0, 0};
    Real offsetX_{0};
    Real offsetY_{0};
};

} // namespace flowopt
