// ======================================================================
// lab9_main.cpp
//
// Лабораторная работа №9: «Контракты с компилятором» (уникальный стиль)
// ======================================================================

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <cassert>

#ifdef _OPENMP
  #include <omp.h>
#endif

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::size_t;
using std::mt19937_64;
using std::uniform_real_distribution;
using std::chrono::high_resolution_clock;
using std::chrono::duration;

// Общее число уравнений для эксперимента
static constexpr size_t ЧИСЛО_УРАВНЕНИЙ = 50'000'000;

// ===================== Часть A. Решение N квадратных уравнений =============

void эксперимент_квадратные()
{
    cout << "=== Часть A: решение квадратных уравнений ===\n\n";

    // 1) Генерация входных данных
    vector<double> коэф_A(ЧИСЛО_УРАВНЕНИЙ), коэф_B(ЧИСЛО_УРАВНЕНИЙ), коэф_C(ЧИСЛО_УРАВНЕНИЙ);
    {
        mt19937_64 генератор(42);
        uniform_real_distribution<double> распр(-1000.0, 1000.0);
        for (size_t i = 0; i < ЧИСЛО_УРАВНЕНИЙ; ++i) {
            double a = распр(генератор);
            while (fabs(a) < 1e-9) a = распр(генератор);
            коэф_A[i] = a;
            коэф_B[i] = распр(генератор);
            коэф_C[i] = распр(генератор);
        }
    }

    enum Режим { 
        ОБЫЧНЫЕ = 0,
        КОНСТ_ВСЕ,
        ВОЛАТ_ВСЕ,
        РЕСТРИКТ_ВСЕ
    };

    size_t граница_цикла_перем = ЧИСЛО_УРАВНЕНИЙ;
    const size_t граница_цикла_конст = ЧИСЛО_УРАВНЕНИЙ;

    auto запуск_варианта = [&](const string &имя,
                               double *a_сырые,
                               double *b_сырые,
                               double *c_сырые,
                               bool    использовать_конст_границу,
                               bool    omp_вкл)
    {
        Режим режим = ОБЫЧНЫЕ;
        if      (имя.rfind("BASELINE",    0) == 0) режим = ОБЫЧНЫЕ;
        else if (имя.rfind("CONST_ALL",   0) == 0) режим = КОНСТ_ВСЕ;
        else if (имя.rfind("VOLATILE_ALL",0) == 0) режим = ВОЛАТ_ВСЕ;
        else if (имя.rfind("RESTRICT_ALL",0) == 0) режим = РЕСТРИКТ_ВСЕ;
        else assert(false && "Неизвестный режим!");

        double *        a = nullptr;
        double *        b = nullptr;
        double *        c = nullptr;
        const double *  a_c = nullptr;
        const double *  b_c = nullptr;
        const double *  c_c = nullptr;
        volatile double *a_v = nullptr;
        volatile double *b_v = nullptr;
        volatile double *c_v = nullptr;
        double * __restrict__ a_r = nullptr;
        double * __restrict__ b_r = nullptr;
        double * __restrict__ c_r = nullptr;

        switch (режим) {
            case ОБЫЧНЫЕ:
                a = a_сырые; b = b_сырые; c = c_сырые; break;
            case КОНСТ_ВСЕ:
                a_c = a_сырые; b_c = b_сырые; c_c = c_сырые; break;
            case ВОЛАТ_ВСЕ:
                a_v = reinterpret_cast<volatile double*>(a_сырые);
                b_v = reinterpret_cast<volatile double*>(b_сырые);
                c_v = reinterpret_cast<volatile double*>(c_сырые); break;
            case РЕСТРИКТ_ВСЕ:
                a_r = a_сырые; b_r = b_сырые; c_r = c_сырые; break;
        }

        size_t кол_корней = 0u;
        auto t_start = high_resolution_clock::now();

        if (omp_вкл)
        {
            #ifdef _OPENMP
              int макс_потоков = omp_get_max_threads();
            #else
              int макс_потоков = 1;
            #endif

            size_t граница = (использовать_конст_границу ? граница_цикла_конст : граница_цикла_перем);
            if (режим == ОБЫЧНЫЕ) {
                #pragma omp parallel for reduction(+:кол_корней)
                for (size_t i = 0; i < граница; ++i) {
                    double a = a[i], b = b[i], c = c[i];
                    double D = b*b - 4.0*a*c;
                    if (D >= 0.0) {
                        double sd = sqrt(D);
                        double x1 = (-b + sd)/(2.0*a);
                        double x2 = (-b - sd)/(2.0*a);
                        (void)x1; (void)x2;
                        кол_корней += 2;
                    }
                }
            } else if (режим == КОНСТ_ВСЕ) {
                #pragma omp parallel for reduction(+:кол_корней)
                for (size_t i = 0; i < граница; ++i) {
                    double a = a_c[i], b = b_c[i], c = c_c[i];
                    double D = b*b - 4.0*a*c;
                    if (D >= 0.0) {
                        double sd = sqrt(D);
                        double x1 = (-b + sd)/(2.0*a);
                        double x2 = (-b - sd)/(2.0*a);
                        (void)x1; (void)x2;
                        кол_корней += 2;
                    }
                }
            } else if (режим == ВОЛАТ_ВСЕ) {
                #pragma omp parallel for reduction(+:кол_корней)
                for (size_t i = 0; i < граница; ++i) {
                    double a = a_v[i], b = b_v[i], c = c_v[i];
                    double D = b*b - 4.0*a*c;
                    if (D >= 0.0) {
                        double sd = sqrt(D);
                        double x1 = (-b + sd)/(2.0*a);
                        double x2 = (-b - sd)/(2.0*a);
                        (void)x1; (void)x2;
                        кол_корней += 2;
                    }
                }
            } else if (режим == РЕСТРИКТ_ВСЕ) {
                #pragma omp parallel for reduction(+:кол_корней)
                for (size_t i = 0; i < граница; ++i) {
                    double a = a_r[i], b = b_r[i], c = c_r[i];
                    double D = b*b - 4.0*a*c;
                    if (D >= 0.0) {
                        double sd = sqrt(D);
                        double x1 = (-b + sd)/(2.0*a);
                        double x2 = (-b - sd)/(2.0*a);
                        (void)x1; (void)x2;
                        кол_корней += 2;
                    }
                }
            }
        }
        else
        {
            size_t граница = (использовать_конст_границу ? граница_цикла_конст : граница_цикла_перем);
            if (режим == ОБЫЧНЫЕ) {
                for (size_t i = 0; i < граница; ++i) {
                    double a = a[i], b = b[i], c = c[i];
                    double D = b*b - 4.0*a*c;
                    if (D >= 0.0) {
                        double sd = sqrt(D);
                        double x1 = (-b + sd)/(2.0*a);
                        double x2 = (-b - sd)/(2.0*a);
                        (void)x1; (void)x2;
                        кол_корней += 2;
                    }
                }
            } else if (режим == КОНСТ_ВСЕ) {
                for (size_t i = 0; i < граница; ++i) {
                    double a = a_c[i], b = b_c[i], c = c_c[i];
                    double D = b*b - 4.0*a*c;
                    if (D >= 0.0) {
                        double sd = sqrt(D);
                        double x1 = (-b + sd)/(2.0*a);
                        double x2 = (-b - sd)/(2.0*a);
                        (void)x1; (void)x2;
                        кол_корней += 2;
                    }
                }
            } else if (режим == ВОЛАТ_ВСЕ) {
                for (size_t i = 0; i < граница; ++i) {
                    double a = a_v[i], b = b_v[i], c = c_v[i];
                    double D = b*b - 4.0*a*c;
                    if (D >= 0.0) {
                        double sd = sqrt(D);
                        double x1 = (-b + sd)/(2.0*a);
                        double x2 = (-b - sd)/(2.0*a);
                        (void)x1; (void)x2;
                        кол_корней += 2;
                    }
                }
            } else if (режим == РЕСТРИКТ_ВСЕ) {
                for (size_t i = 0; i < граница; ++i) {
                    double a = a_r[i], b = b_r[i], c = c_r[i];
                    double D = b*b - 4.0*a*c;
                    if (D >= 0.0) {
                        double sd = sqrt(D);
                        double x1 = (-b + sd)/(2.0*a);
                        double x2 = (-b - sd)/(2.0*a);
                        (void)x1; (void)x2;
                        кол_корней += 2;
                    }
                }
            }
        }

        auto t_end = high_resolution_clock::now();
        double затрачено = duration<double>(t_end - t_start).count();

        cout << "Вариант: " << имя
             << " | граница=" << (использовать_конст_границу ? "const" : "var")
             << " | OpenMP=" << (omp_вкл ? "ON" : "OFF")
             << " | Время=" << затрачено << " с"
             << " | Корней=" << кол_корней
             << "\n";
    };

    vector<string> режимы = { "BASELINE", "CONST_ALL", "VOLATILE_ALL", "RESTRICT_ALL" };
    for (const auto &реж : режимы) {
        for (bool use_c : { false, true }) {
            for (bool omp_on : { false, true }) {
                string имя = реж + (omp_on ? "_OMP" : "_NOOMP");
                запуск_варианта(имя, коэф_A.data(), коэф_B.data(), коэф_C.data(), use_c, omp_on);
            }
        }
    }
}

// ===================== Часть B. «Бесполезная сумма» =========================

void эксперимент_бесполезная_сумма()
{
    cout << "\n=== Часть B: бесполезная сумма (volatile vs non-volatile) ===\n\n";

    static constexpr size_t М = 200'000'000;
    double *массив = new double[М];
    for (size_t i = 0; i < М; ++i) {
        массив[i] = 1.0;
    }

    // 1) Не volatile
    {
        double сумма = 0.0;
        auto t0 = high_resolution_clock::now();
        for (size_t i = 0; i < М; ++i) {
            сумма += массив[i];
        }
        auto t1 = high_resolution_clock::now();
        double t = duration<double>(t1 - t0).count();
        cout << "Сумма (non-volatile):\t время = " << t << " с"
             << " | снимок = " << сумма << "\n";
    }

    // 2) С volatile
    {
        volatile double сумма2 = 0.0;
        auto t0 = high_resolution_clock::now();
        for (size_t i = 0; i < М; ++i) {
            сумма2 += массив[i];
        }
        auto t1 = high_resolution_clock::now();
        double t = duration<double>(t1 - t0).count();
        cout << "Сумма (volatile):\t время = " << t << " с"
             << " | снимок = " << сумма2 << "\n";
    }

    delete [] массив;
}

// ======================================================================
// main
// ======================================================================

int main()
{
    cout << "\nЛабораторная №9: контракты с компилятором (OpenMP)\n";
    cout << "====================================================\n\n";

    // Часть A: квадратные уравнения
    эксперимент_квадратные();

    // Часть B: бесполезная сумма
    эксперимент_бесполезная_сумма();

    return 0;
}
