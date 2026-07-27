#pragma once

#include "flowopt/sim/World.hpp"

#include <string>

namespace flowopt {

// Laedt ein Szenario (Netz + Nachfrage) und erzeugt eine startklare World.
// Format spaeter JSON/CSV; hier zusaetzlich ein eingebautes Demo-Netz, damit
// das Grundgeruest sofort lauffaehig ist.
class ScenarioLoader {
public:
    // Laedt aus einer Datei. Wirft bei ungueltigem Pfad/Format (spaeter).
    [[nodiscard]] static World fromFile(const std::string& path);

    // Kleines, fest verdrahtetes Demo-Netz (Gitter aus wenigen Kreuzungen).
    [[nodiscard]] static World demoGrid();
};

} // namespace flowopt
