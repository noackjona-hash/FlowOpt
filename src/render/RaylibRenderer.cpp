#include "flowopt/render/RaylibRenderer.hpp"

#include <raylib.h>

#include <algorithm>
#include <cmath>

namespace flowopt {

namespace {
constexpr Real kMargin     = Real(40);   // Rand in Pixeln
constexpr Real kLaneWidth  = Real(3.5);  // Fahrspurbreite [m]
constexpr Real kLaneOffset = Real(2.2);  // seitlicher Versatz je Richtung [m]
constexpr Real kRad2Deg    = Real(57.2957795);

[[nodiscard]] Vector2 rv(Vec2 p) noexcept { return Vector2{p.x, p.y}; }

// Lineare Farbmischung (fuer Geschwindigkeits-Kodierung der Fahrzeuge).
[[nodiscard]] Color mixColor(Color a, Color b, Real t) noexcept {
    const Real s = clampReal(t, Real(0), Real(1));
    auto mix = [&](unsigned char x, unsigned char y) {
        return static_cast<unsigned char>(x + (y - x) * s);
    };
    return Color{mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b), 255};
}

// Nach rechts zeigender Einheits-Normalenvektor zur Fahrtrichtung (Welt-Raum).
[[nodiscard]] Vec2 rightNormal(Vec2 from, Vec2 to) noexcept {
    const Vec2 d = to - from;
    const Real len = length(d);
    if (len < Real(1e-4)) {
        return Vec2{0, 0};
    }
    return Vec2{d.y / len, -d.x / len};
}
} // namespace

RaylibRenderer::RaylibRenderer(int width, int height, const char* title)
    : width_(width), height_(height) {
    InitWindow(width_, height_, title);
    SetTargetFPS(60);
}

RaylibRenderer::~RaylibRenderer() {
    CloseWindow();
}

void RaylibRenderer::fitToNetwork(const World& world) {
    const RoadNetwork& net = world.net;
    if (net.nodeCount() == 0) {
        transformReady_ = true;
        return;
    }

    Vec2 lo = net.nodePos[0];
    Vec2 hi = net.nodePos[0];
    for (const Vec2 p : net.nodePos) {
        lo.x = std::min(lo.x, p.x);
        lo.y = std::min(lo.y, p.y);
        hi.x = std::max(hi.x, p.x);
        hi.y = std::max(hi.y, p.y);
    }
    worldMin_ = lo;

    const Real worldW = std::max(hi.x - lo.x, Real(1));
    const Real worldH = std::max(hi.y - lo.y, Real(1));
    const Real availW = static_cast<Real>(width_) - Real(2) * kMargin;
    const Real availH = static_cast<Real>(height_) - Real(2) * kMargin;

    scale_ = std::min(availW / worldW, availH / worldH);
    offsetX_ = (static_cast<Real>(width_) - worldW * scale_) * Real(0.5);
    offsetY_ = (static_cast<Real>(height_) - worldH * scale_) * Real(0.5);
    transformReady_ = true;
}

Vec2 RaylibRenderer::worldToScreen(Vec2 p) const {
    const Real sx = (p.x - worldMin_.x) * scale_ + offsetX_;
    const Real sy = (p.y - worldMin_.y) * scale_ + offsetY_;
    return Vec2{sx, static_cast<Real>(height_) - sy};  // y spiegeln (Bildschirm zeigt nach unten)
}

void RaylibRenderer::draw(const World& world) {
    if (!transformReady_) {
        fitToNetwork(world);
    }

    const RoadNetwork& net = world.net;
    const VehiclePool& veh = world.vehicles;

    BeginDrawing();
    ClearBackground(Color{40, 44, 52, 255});

    // --- Fahrbahnen: je Kante ein graues Asphaltband, nach rechts versetzt ---
    const Real bandPx = std::max(kLaneWidth * scale_, Real(2));
    for (std::size_t e = 0; e < net.edgeCount(); ++e) {
        const Vec2 a = net.nodePos[idx(net.edgeFrom[e])];
        const Vec2 b = net.nodePos[idx(net.edgeTo[e])];
        const Vec2 r = rightNormal(a, b);
        const Vec2 aOff = a + r * kLaneOffset;
        const Vec2 bOff = b + r * kLaneOffset;
        DrawLineEx(rv(worldToScreen(aOff)), rv(worldToScreen(bOff)), bandPx, Color{70, 74, 82, 255});
        // Duenne Fahrstreifenbegrenzung.
        DrawLineEx(rv(worldToScreen(aOff)), rv(worldToScreen(bOff)), std::max(bandPx * Real(0.08), Real(1)),
                   Color{120, 124, 132, 255});
    }

    // --- Kreuzungen als Knotenpunkte ---
    const Real nodeR = std::max(kLaneWidth * scale_ * Real(0.7), Real(4));
    for (std::size_t n = 0; n < net.nodeCount(); ++n) {
        DrawCircleV(rv(worldToScreen(net.nodePos[n])), nodeR, Color{90, 94, 102, 255});
    }

    // --- Ampeln: Farbindikator an der Haltelinie jeder Zufahrtsspur ---
    for (std::size_t l = 0; l < net.laneCount(); ++l) {
        const EdgeId e = net.laneEdge[l];
        const Vec2 a = net.nodePos[idx(net.edgeFrom[idx(e)])];
        const Vec2 b = net.nodePos[idx(net.edgeTo[idx(e)])];
        const Vec2 r = rightNormal(a, b);
        const Vec2 stop = lerp(a, b, Real(0.9)) + r * kLaneOffset;

        Color c;
        switch (world.laneSignal[l]) {
            case SignalState::Green: c = Color{60, 200, 80, 255};  break;
            case SignalState::Amber: c = Color{235, 200, 40, 255}; break;
            case SignalState::Red:   c = Color{230, 60, 50, 255};  break;
        }
        DrawCircleV(rv(worldToScreen(stop)), std::max(bandPx * Real(0.28), Real(3)), c);
    }

    // --- Fahrzeuge: Rechtecke, eingefaerbt nach Geschwindigkeit ---
    const Color slow{230, 60, 50, 255};   // Rot = Stau/Halt
    const Color fast{60, 200, 80, 255};   // Gruen = freie Fahrt
    for (std::uint32_t s = 0; s < veh.capacity(); ++s) {
        if (!veh.active[s]) {
            continue;
        }
        const std::uint32_t l = idx(veh.lane[s]);
        const EdgeId e = net.laneEdge[l];
        const Vec2 a = net.nodePos[idx(net.edgeFrom[idx(e)])];
        const Vec2 b = net.nodePos[idx(net.edgeTo[idx(e)])];
        const Real laneLen = std::max(net.laneLength[l], Real(1e-3));
        const Real t = clampReal(veh.posOnLane[s] / laneLen, Real(0), Real(1));
        const Vec2 r = rightNormal(a, b);
        const Vec2 posW = lerp(a, b, t) + r * kLaneOffset;

        const Vec2 pS = worldToScreen(posW);
        const Vec2 aS = worldToScreen(a);
        const Vec2 bS = worldToScreen(b);
        const Real angle = std::atan2(bS.y - aS.y, bS.x - aS.x) * kRad2Deg;

        const Real ratio = veh.v0[s] > Real(0) ? veh.speed[s] / veh.v0[s] : Real(0);
        const Color col = mixColor(slow, fast, ratio);

        const Real lenPx = std::max(veh.length[s] * scale_, Real(4));
        const Real widPx = std::max(Real(2) * scale_, Real(3));
        const Rectangle rec{pS.x, pS.y, lenPx, widPx};
        DrawRectanglePro(rec, Vector2{lenPx * Real(0.5), widPx * Real(0.5)}, angle, col);
    }

    // --- HUD ---
    DrawText(TextFormat("t = %.1f s   step %llu", world.time,
                        static_cast<unsigned long long>(world.stepIndex)),
             10, 10, 20, RAYWHITE);
    DrawText(TextFormat("vehicles: %zu", veh.activeCount()), 10, 34, 20, RAYWHITE);

    EndDrawing();
}

bool RaylibRenderer::shouldClose() {
    return WindowShouldClose();
}

} // namespace flowopt
