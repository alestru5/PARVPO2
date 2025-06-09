#include <iostream>
#include <cmath>
#include <random>
#include <chrono>
#include <tuple>
#include <optional>

#define ITERATIONS 100000000
#define RANDOM_SEED 42

inline double calcDelta(double alpha, double beta, double gamma) {
    return beta * beta - 4 * alpha * gamma;
}

inline double findRootA(double alpha, double beta, double delta) {
    return (-beta + std::sqrt(delta)) / (2 * alpha);
}

inline double findRootB(double alpha, double beta, double delta) {
    return (-beta - std::sqrt(delta)) / (2 * alpha);
}

inline std::optional<std::tuple<double, double>> getRoots(double alpha, double beta, double gamma) {
    double delta = calcDelta(alpha, beta, gamma);
    if (delta < 0) return std::nullopt;
    double rA = findRootA(alpha, beta, delta);
    double rB = findRootB(alpha, beta, delta);
    return (delta == 0) ? std::make_tuple(rA, rA) : std::make_tuple(rA, rB);
}

int main() {
    std::mt19937 generator(RANDOM_SEED);
    std::uniform_real_distribution<double> coeffGen(-100.0, 100.0);
    int realRootsCount = 0;
    auto tStart = std::chrono::high_resolution_clock::now();
    for (int idx = 0; idx < ITERATIONS; ++idx) {
        double alpha = coeffGen(generator);
        double beta = coeffGen(generator);
        double gamma = coeffGen(generator);
        auto roots = getRoots(alpha, beta, gamma);
        if (roots) realRootsCount++;
    }
    auto tEnd = std::chrono::high_resolution_clock::now();
    std::cout << "Время выполнения: " << std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count() << " мс\n";
    std::cout << "Количество уравнений с действительными корнями: " << realRootsCount << std::endl;
    return 0;
} 