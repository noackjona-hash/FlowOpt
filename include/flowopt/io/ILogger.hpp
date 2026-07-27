#pragma once

#include "flowopt/core/Types.hpp"

#include <iostream>
#include <string_view>

namespace flowopt {

// Logging-Schnittstelle. Der Core kennt nur dieses Interface -> Headless nutzt
// den NullLogger (Zero-Overhead), Debug/GUI einen ausfuehrlichen Logger.
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void metric(std::string_view key, double value, Real simTime) = 0;
    virtual void message(std::string_view text) = 0;
};

// Verwirft alles -- Standard fuer Batch-Training.
class NullLogger final : public ILogger {
public:
    void metric(std::string_view, double, Real) override {}
    void message(std::string_view) override {}
};

// Schreibt nach stdout -- fuer Entwicklung und GUI.
class ConsoleLogger final : public ILogger {
public:
    void metric(std::string_view key, double value, Real simTime) override {
        std::cout << "[t=" << simTime << "] " << key << " = " << value << '\n';
    }
    void message(std::string_view text) override {
        std::cout << text << '\n';
    }
};

} // namespace flowopt
