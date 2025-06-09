#ifndef ROOTS_LIB_H
#define ROOTS_LIB_H

#include <tuple>
#include <optional>

std::optional<std::tuple<double, double>> getRoots(double alpha, double beta, double gamma);

double calcDelta(double alpha, double beta, double gamma);
double findRootA(double alpha, double beta, double delta);
double findRootB(double alpha, double beta, double delta);

#endif 