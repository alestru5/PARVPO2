#include <cmath>
#include <tuple>
#include <optional>
#include "roots_lib.h"

double calcDelta(double alpha, double beta, double gamma) {
    return beta * beta - 4 * alpha * gamma;
}

double findRootA(double alpha, double beta, double delta) {
    return (-beta + std::sqrt(delta)) / (2 * alpha);
}

double findRootB(double alpha, double beta, double delta) {
    return (-beta - std::sqrt(delta)) / (2 * alpha);
}

std::optional<std::tuple<double, double>> getRoots(double alpha, double beta, double gamma) {
    double delta = calcDelta(alpha, beta, gamma);
    if (delta < 0) return std::nullopt;
    double rA = findRootA(alpha, beta, delta);
    double rB = findRootB(alpha, beta, delta);
    return (delta == 0) ? std::make_tuple(rA, rA) : std::make_tuple(rA, rB);
} 