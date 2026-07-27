#pragma once

#include "flowopt/sim/World.hpp"

namespace flowopt {

// Abstrakte Render-Senke. Der Core haengt nur von diesem Interface ab, nie von
// einer konkreten Grafikbibliothek. Eine Implementierung (z.B. RaylibRenderer)
// wird nur im GUI-Build kompiliert und greift ausschliesslich LESEND auf World zu.
class IRenderer {
public:
    virtual ~IRenderer() = default;

    // Zeichnet den aktuellen Weltzustand (darf ihn nicht veraendern).
    virtual void draw(const World& world) = 0;

    // true, wenn das Fenster geschlossen werden soll (beendet den GUI-Loop).
    [[nodiscard]] virtual bool shouldClose() = 0;
};

} // namespace flowopt
