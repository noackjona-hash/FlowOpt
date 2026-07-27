#include "flowopt/render/RaylibRenderer.hpp"

#include <raylib.h>

namespace flowopt {

RaylibRenderer::RaylibRenderer(int width, int height, const char* title) {
    InitWindow(width, height, title);
    SetTargetFPS(60);
}

RaylibRenderer::~RaylibRenderer() {
    CloseWindow();
}

void RaylibRenderer::draw(const World& world) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Kreuzungen zeichnen.
    for (std::size_t n = 0; n < world.net.nodeCount(); ++n) {
        const Vec2 p = world.net.nodePos[n];
        DrawCircle(static_cast<int>(p.x * pixelsPerMeter_) + 100,
                   static_cast<int>(p.y * pixelsPerMeter_) + 360, 6.0f, DARKGRAY);
    }

    // TODO: Kanten/Lanes und Fahrzeuge aus dem World-Zustand rendern.

    EndDrawing();
}

bool RaylibRenderer::shouldClose() {
    return WindowShouldClose();
}

} // namespace flowopt
