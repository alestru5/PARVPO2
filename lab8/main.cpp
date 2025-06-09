// lab8_main.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <fstream>

#define КОЛ_УРАВНЕНИЙ 50'000'000
#define ЗЕРНО_ГЕНЕРАТОРА 42

using std::cout;
using std::endl;
using std::vector;
using std::fabs;
using std::mt19937;
using std::uniform_real_distribution;
using std::chrono::high_resolution_clock;
using std::chrono::duration;

// Результат решения квадратного уравнения
struct РезультатУравнения {
    double коэф_a, коэф_b, коэф_c;
    int число_корней;
    double корень1, корень2;
};

// Решение квадратного уравнения: возвращает число корней и сами корни
int решить_квадратное(double a, double b, double c, double &x1, double &x2) {
    double дискриминант = b * b - 4 * a * c;
    if (дискриминант > 0) {
        double sqrt_d = sqrt(дискриминант);
        x1 = (-b + sqrt_d) / (2 * a);
        x2 = (-b - sqrt_d) / (2 * a);
        return 2;
    } else if (дискриминант == 0) {
        x1 = x2 = -b / (2 * a);
        return 1;
    } else {
        x1 = x2 = 0;
        return 0;
    }
}

int main() {
    mt19937 генератор(ЗЕРНО_ГЕНЕРАТОРА);
    uniform_real_distribution<double> распределение(-1000.0, 1000.0);
    vector<РезультатУравнения> решения;
    решения.reserve(КОЛ_УРАВНЕНИЙ);
    auto время_старта = high_resolution_clock::now();
    for (size_t i = 0; i < КОЛ_УРАВНЕНИЙ; ++i) {
        double a = распределение(генератор);
        while (fabs(a) < 1e-6) a = распределение(генератор);
        double b = распределение(генератор);
        double c = распределение(генератор);
        double x1, x2;
        int корней = решить_квадратное(a, b, c, x1, x2);
        решения.push_back({a, b, c, корней, x1, x2});
    }
    auto время_конца = high_resolution_clock::now();
    double сек_затрачено = duration<double>(время_конца - время_старта).count();
    cout << "Решено " << КОЛ_УРАВНЕНИЙ << " уравнений за " << сек_затрачено << " секунд." << endl;
    return 0;
}
