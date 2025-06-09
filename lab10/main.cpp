#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cstdlib>

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || (defined(__INTEL_COMPILER)) || defined(__clang__)
    #define вероятно(expr)   (__builtin_expect(static_cast<bool>(expr), true))
    #define маловероятно(expr) (__builtin_expect(static_cast<bool>(expr), false))
#else
    #define вероятно(expr)   (expr)
    #define маловероятно(expr) (expr)
#endif

using std::cout;
using std::endl;
using std::vector;
using std::size_t;
using std::mt19937_64;
using std::uniform_int_distribution;
using std::chrono::high_resolution_clock;
using std::chrono::duration;

static const size_t РАЗМЕР_МАССИВА = 500'000'000;
static const int ПОВТОРОВ = 3;

// Заполнение массива случайными числами
void заполнить_массив(vector<int>& массив) {
    mt19937_64 генератор(42);
    uniform_int_distribution<int> распр(1, 1000);
    for (size_t i = 0; i < массив.size(); ++i) {
        массив[i] = распр(генератор);
    }
}

// Базовая сумма без подсказок
void базовая_сумма(const vector<int>& массив, long long& часто, long long& редко) {
    часто = 0;
    редко = 0;
    for (size_t i = 0; i < массив.size(); ++i) {
        if (i % 1000 == 0) {
            редко += массив[i];
        } else {
            часто += массив[i];
        }
    }
}

// Сумма с правильной подсказкой (маловероятно)
void верная_подсказка(const vector<int>& массив, long long& часто, long long& редко) {
    часто = 0;
    редко = 0;
    for (size_t i = 0; i < массив.size(); ++i) {
        if (маловероятно(i % 1000 == 0)) {
            редко += массив[i];
        } else {
            часто += массив[i];
        }
    }
}

// Сумма с неправильной подсказкой (вероятно)
void неверная_подсказка(const vector<int>& массив, long long& часто, long long& редко) {
    часто = 0;
    редко = 0;
    for (size_t i = 0; i < массив.size(); ++i) {
        if (вероятно(i % 1000 == 0)) {
            редко += массив[i];
        } else {
            часто += массив[i];
        }
    }
}

// Сумма с инвертированной подсказкой
void инверт_подсказка(const vector<int>& массив, long long& часто, long long& редко) {
    часто = 0;
    редко = 0;
    for (size_t i = 0; i < массив.size(); ++i) {
        if (!(маловероятно(i % 1000 != 0))) {
            редко += массив[i];
        } else {
            часто += массив[i];
        }
    }
}

// Измерение времени выполнения функции
template<typename Функция>
double измерить_время(Функция f, const vector<int>& массив, long long& out_часто, long long& out_редко) {
    double всего = 0.0;
    long long sm = 0, sr = 0;
    for (int r = 0; r < ПОВТОРОВ; ++r) {
        auto старт = high_resolution_clock::now();
        f(массив, sm, sr);
        auto конец = high_resolution_clock::now();
        всего += duration<double>(конец - старт).count();
    }
    out_часто = sm;
    out_редко = sr;
    return всего / ПОВТОРОВ;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    cout << "=======================================================\n\n";
    vector<int> данные(РАЗМЕР_МАССИВА);
    заполнить_массив(данные);
    cout << "Размер массива " << РАЗМЕР_МАССИВА << " элементов\n";
    long long баз_часто = 0, баз_редко = 0;
    long long верн_часто  = 0, верн_редко  = 0;
    long long неверн_часто    = 0, неверн_редко    = 0;
    long long инверт_часто   = 0, инверт_редко   = 0;

    double t_base = измерить_время(базовая_сумма,       данные, баз_часто, баз_редко);
    double t_true  = измерить_время(верная_подсказка,  данные, верн_часто,  верн_редко);
    double t_false    = измерить_время(неверная_подсказка,    данные, неверн_часто,    неверн_редко);
    double t_inv   = измерить_время(инверт_подсказка, данные, инверт_часто,   инверт_редко);

    cout << "=== Результаты (усреднённое время из " << ПОВТОРОВ << " запусков) ===\n\n";
    cout << "1) Базовый вариант (без подсказок):\n";
    cout << "   Время = " << t_base << " с,  часто = " << баз_часто
         << ", редко = " << баз_редко << "\n\n";
    cout << "2) Верная подсказка (маловероятно(i%1000==0)):\n";
    cout << "   Время = " << t_true << " с,  часто = " << верн_часто
         << ", редко = " << верн_редко << "\n\n";
    cout << "3) Неверная подсказка (вероятно(i%1000==0)):\n";
    cout << "   Время = " << t_false << " с,  часто = " << неверн_часто
         << ", редко = " << неверн_редко << "\n\n";
    cout << "4) Инвертированная подсказка:\n";
    cout << "   Время = " << t_inv << " с,  часто = " << инверт_часто
         << ", редко = " << инверт_редко << "\n\n";
    cout << "=======================================================\n";
    return 0;
}

