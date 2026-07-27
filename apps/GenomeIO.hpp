#pragma once

#include "flowopt/core/Types.hpp"

#include <cstdio>
#include <fstream>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace flowopt {

// Serialisiert ein Chromosom (Controller-Parametervektor) als schlankes JSON.
inline bool saveGenome(const std::string& path, std::span<const Real> genome,
                       double fitness, const std::string& scenario) {
    std::ofstream out(path);
    if (!out) {
        return false;
    }
    out << "{\n  \"scenario\": \"" << scenario << "\",\n";
    out << "  \"fitness\": " << fitness << ",\n";
    out << "  \"parameters\": [";
    for (std::size_t i = 0; i < genome.size(); ++i) {
        if (i) out << ", ";
        out << genome[i];
    }
    out << "]\n}\n";
    return true;
}

// Liest die Parameterliste aus einer mit saveGenome erzeugten Datei.
// Robuster Minimal-Parser: extrahiert die Zahlen zwischen dem ersten '[' und ']'.
inline std::vector<Real> loadGenome(const std::string& path) {
    std::vector<Real> genome;
    std::ifstream in(path);
    if (!in) {
        return genome;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    const std::string text = buffer.str();

    const auto begin = text.find('[');
    const auto end = text.find(']', begin);
    if (begin == std::string::npos || end == std::string::npos) {
        return genome;
    }

    std::string body = text.substr(begin + 1, end - begin - 1);
    for (char& ch : body) {
        if (ch == ',') ch = ' ';
    }
    std::stringstream values(body);
    Real v;
    while (values >> v) {
        genome.push_back(v);
    }
    return genome;
}

} // namespace flowopt
