#include "AppCommon.hpp"
#include "GenomeIO.hpp"

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <vector>

// GA-Trainer: optimiert die Gewichte der PolicyController automatisch. Eine
// Population von Chromosomen (flache Parametervektoren) wird headless gegen
// dieselbe Verkehrssituation (fester Seed) bewertet; die besten pflanzen sich
// per Elitismus, k-Punkt-Crossover und Gauss-Mutation fort.
using namespace flowopt;

namespace {

struct GAConfig {
    int           generations  = 40;
    int           population   = 32;
    std::uint64_t steps        = 5000;
    std::uint64_t simSeed      = 42;       // fester Sim-Seed -> faire, vergleichbare Bewertung
    std::uint64_t gaSeed       = 1234;     // Seed fuer den GA selbst
    std::string   scenario;
    std::string   out          = "best_policy.json";
    double        eliteFrac    = 0.10;
    double        mutationRate  = 0.15;
    float         mutationSigma = 0.20f;
};

// Baut eine frische Sim, setzt das Chromosom und liefert die Fitness (kleiner = besser).
double evaluate(SimConfig base, std::span<const Real> genome) {
    Simulation sim = buildSimulation(base);
    sim.setParameters(genome);
    sim.run(base.steps);
    return sim.metrics().fitness();
}

// Referenzwert eines bestimmten Controller-Typs ohne Optimierung.
double baseline(SimConfig base, ControllerKind kind) {
    base.controller = kind;
    base.policyPath.clear();
    Simulation sim = buildSimulation(base);
    sim.run(base.steps);
    return sim.metrics().fitness();
}

std::uint64_t toU64(std::string_view s, std::uint64_t def) {
    std::uint64_t v = def;
    std::from_chars(s.data(), s.data() + s.size(), v);
    return v;
}

double toF(std::string_view s, double def) {
    try { return std::stod(std::string(s)); } catch (...) { return def; }
}

GAConfig parseGa(int argc, char** argv) {
    GAConfig g;
    for (int i = 1; i < argc; ++i) {
        std::string_view a = argv[i];
        auto next = [&]() -> std::string_view {
            return (i + 1 < argc) ? std::string_view{argv[++i]} : std::string_view{};
        };
        if (a == "--generations")        g.generations = static_cast<int>(toU64(next(), g.generations));
        else if (a == "--population")    g.population = static_cast<int>(toU64(next(), g.population));
        else if (a == "--steps")         g.steps = toU64(next(), g.steps);
        else if (a == "--seed")          g.simSeed = toU64(next(), g.simSeed);
        else if (a == "--ga-seed")       g.gaSeed = toU64(next(), g.gaSeed);
        else if (a == "--scenario")      g.scenario = std::string(next());
        else if (a == "--out")           g.out = std::string(next());
        else if (a == "--elite-frac")    g.eliteFrac = toF(next(), g.eliteFrac);
        else if (a == "--mutation-rate") g.mutationRate = toF(next(), g.mutationRate);
        else if (a == "--mutation-sigma") g.mutationSigma = static_cast<float>(toF(next(), g.mutationSigma));
    }
    return g;
}

} // namespace

int main(int argc, char** argv) {
    const GAConfig ga = parseGa(argc, argv);

    SimConfig base;
    base.controller = ControllerKind::Policy;
    base.steps = ga.steps;
    base.seed = ga.simSeed;
    base.scenarioPath = ga.scenario;

    // Genom-Laenge = Gesamtzahl der Controller-Parameter des Szenarios.
    const std::size_t G = buildSimulation(base).parameterCount();
    if (G == 0) {
        std::printf("Keine optimierbaren Parameter (PolicyController liefert 0). Abbruch.\n");
        return 1;
    }

    std::mt19937_64 rng(ga.gaSeed);
    std::uniform_real_distribution<float> initDist(0.0f, 1.0f);
    std::uniform_int_distribution<std::size_t> geneDist(0, G - 1);
    std::normal_distribution<float> gauss(0.0f, ga.mutationSigma);
    std::uniform_int_distribution<int> popDist(0, ga.population - 1);

    // --- Population initialisieren ---
    std::vector<std::vector<Real>> pop(ga.population, std::vector<Real>(G));
    for (auto& ind : pop) {
        for (auto& gene : ind) gene = initDist(rng);
    }
    std::vector<double> fit(ga.population, 0.0);

    std::vector<Real> best(G);
    double bestFit = std::numeric_limits<double>::max();

    const int eliteCount = std::max(1, static_cast<int>(ga.population * ga.eliteFrac));

    auto tournament = [&](void) -> const std::vector<Real>& {
        int a = popDist(rng), b = popDist(rng), c = popDist(rng);
        int win = a;
        if (fit[b] < fit[win]) win = b;
        if (fit[c] < fit[win]) win = c;
        return pop[win];
    };

    std::printf("GA-Training: genome=%zu  pop=%d  gen=%d  steps=%llu\n",
                G, ga.population, ga.generations,
                static_cast<unsigned long long>(ga.steps));

    for (int gen = 0; gen < ga.generations; ++gen) {
        // --- Bewertung ---
        double sum = 0.0;
        for (int i = 0; i < ga.population; ++i) {
            fit[i] = evaluate(base, pop[i]);
            sum += fit[i];
            if (fit[i] < bestFit) {
                bestFit = fit[i];
                best = pop[i];
            }
        }

        // --- Rangfolge (aufsteigend, kleiner = besser) ---
        std::vector<int> order(ga.population);
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int x, int y) { return fit[x] < fit[y]; });

        std::printf("  gen %3d  best %.1f  mean %.1f\n", gen, fit[order[0]],
                    sum / ga.population);

        // --- Naechste Generation ---
        std::vector<std::vector<Real>> next;
        next.reserve(ga.population);

        // Elitismus: beste Individuen unveraendert uebernehmen.
        for (int i = 0; i < eliteCount; ++i) {
            next.push_back(pop[order[i]]);
        }

        // Rest per Selektion + 2-Punkt-Crossover + Gauss-Mutation.
        while (static_cast<int>(next.size()) < ga.population) {
            const std::vector<Real>& pa = tournament();
            const std::vector<Real>& pb = tournament();

            std::size_t c1 = geneDist(rng), c2 = geneDist(rng);
            if (c1 > c2) std::swap(c1, c2);

            std::vector<Real> child(G);
            for (std::size_t k = 0; k < G; ++k) {
                child[k] = (k >= c1 && k < c2) ? pb[k] : pa[k];
            }
            for (std::size_t k = 0; k < G; ++k) {
                if (initDist(rng) < ga.mutationRate) {
                    child[k] = std::clamp(child[k] + gauss(rng), Real(-2), Real(2));
                }
            }
            next.push_back(std::move(child));
        }
        pop.swap(next);
    }

    // --- Referenzwerte und Export ---
    const double fixedRef = baseline(base, ControllerKind::FixedTime);
    const double ruleRef  = baseline(base, ControllerKind::RuleBased);

    std::printf("\nErgebnis:\n");
    std::printf("  FixedTime fitness : %.1f\n", fixedRef);
    std::printf("  RuleBased fitness : %.1f\n", ruleRef);
    std::printf("  Policy(GA) fitness: %.1f\n", bestFit);

    if (saveGenome(ga.out, best, bestFit, ga.scenario.empty() ? "demoGrid" : ga.scenario)) {
        std::printf("  bestes Chromosom -> %s\n", ga.out.c_str());
    } else {
        std::printf("  FEHLER: konnte %s nicht schreiben\n", ga.out.c_str());
        return 1;
    }
    return 0;
}
