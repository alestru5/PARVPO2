#include <iostream>
#include <random>
#include <chrono>
#include "roots_lib.h"

#define ITERATIONS 100000000
#define RANDOM_SEED 42

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