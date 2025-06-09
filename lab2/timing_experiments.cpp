#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <x86intrin.h>
#include <ctime>
#include <numeric>
#include <algorithm>
#include <stdexcept>

using namespace std;

constexpr size_t ИТЕРАЦИЙ = 1'000'000;
constexpr size_t ОБРАЗЦОВ_RTC = 100;
constexpr size_t ОБРАЗЦОВ_КАЛИБР = 100;
constexpr int МАКС_ПОПЫТОК_RTC = 1000;

struct ХарактеристикиТаймера {
    double разрешение;
    double ошибка_разрешения;
    double системная_точность;
    double время_инициализации;
    double время_ответа;
};

class ИзмерительТаймеров {
    double tsc_на_нс;
    vector<clockid_t> поддерживаемые_часы;

    void калибровать_tsc() {
        vector<double> коэффициенты;
        for (size_t i = 0; i < ОБРАЗЦОВ_КАЛИБР; ++i) {
            timespec старт, стоп;
            uint64_t tsc1 = __rdtsc();
            clock_gettime(CLOCK_MONOTONIC_RAW, &старт);
            usleep(1000);
            clock_gettime(CLOCK_MONOTONIC_RAW, &стоп);
            uint64_t tsc2 = __rdtsc();
            double нс = (стоп.tv_sec - старт.tv_sec)*1e9 + (стоп.tv_nsec - старт.tv_nsec);
            коэффициенты.push_back((tsc2 - tsc1) / нс);
        }
        tsc_на_нс = accumulate(коэффициенты.begin(), коэффициенты.end(), 0.0) / коэффициенты.size();
    }

    double tsc_в_нс(uint64_t циклы) const {
        return циклы / tsc_на_нс;
    }

    template<typename Clock>
    ХарактеристикиТаймера измерить_часы_cpp() {
        vector<uint64_t> дельты, инициализации, ответа;
        for (size_t i = 0; i < ИТЕРАЦИЙ; ++i) {
            uint64_t t1 = __rdtsc();
            auto now = Clock::now();
            uint64_t t2 = __rdtsc();
            auto now2 = Clock::now();
            uint64_t t3 = __rdtsc();
            дельты.push_back(t2 - t1);
            инициализации.push_back(t2 - t1);
            ответа.push_back(t3 - t2);
            asm volatile("" : : "r"(now), "r"(now2) : "memory");
        }
        double среднее = accumulate(дельты.begin(), дельты.end(), 0.0) / ИТЕРАЦИЙ;
        double sq_sum = inner_product(дельты.begin(), дельты.end(), дельты.begin(), 0.0);
        double отклонение = sqrt(sq_sum/ИТЕРАЦИЙ - среднее*среднее);
        constexpr clockid_t clk =
            is_same_v<Clock, chrono::system_clock> ? CLOCK_REALTIME :
            is_same_v<Clock, chrono::steady_clock> ? CLOCK_MONOTONIC :
            CLOCK_MONOTONIC_RAW;
        timespec res;
        clock_getres(clk, &res);
        double sys_acc = res.tv_sec*1e9 + res.tv_nsec;
        double a1 = accumulate(инициализации.begin(), инициализации.end(), 0.0) / ИТЕРАЦИЙ;
        double a2 = accumulate(ответа.begin(), ответа.end(), 0.0) / ИТЕРАЦИЙ;
        return {
            tsc_в_нс(среднее),
            tsc_в_нс(отклонение),
            sys_acc,
            tsc_в_нс(a1),
            tsc_в_нс(a2)
        };
    }

public:
    ИзмерительТаймеров() {
        калибровать_tsc();
        поддерживаемые_часы = {
            CLOCK_REALTIME,
            CLOCK_MONOTONIC,
            CLOCK_MONOTONIC_RAW,
            CLOCK_PROCESS_CPUTIME_ID,
            CLOCK_THREAD_CPUTIME_ID
        };
    }

    ХарактеристикиТаймера измерить_rtc() {
        int fd = open("/dev/rtc0", O_RDONLY);
        if (fd < 0) throw runtime_error("Ошибка открытия RTC");
        vector<uint64_t> инициализации, ответа, дельты;
        rtc_time prev, curr;
        if (ioctl(fd, RTC_RD_TIME, &prev) < 0) {
            close(fd);
            throw runtime_error("Ошибка чтения RTC");
        }
        for (size_t i = 0; i < ОБРАЗЦОВ_RTC; ++i) {
            int попытки = 0;
            uint64_t t1 = __rdtsc();
            if (ioctl(fd, RTC_RD_TIME, &curr) < 0) {
                close(fd);
                throw runtime_error("Ошибка чтения RTC");
            }
            uint64_t t2 = __rdtsc();
            инициализации.push_back(t2 - t1);
            uint64_t t3 = __rdtsc();
            if (ioctl(fd, RTC_RD_TIME, &prev) < 0) {
                close(fd);
                throw runtime_error("Ошибка чтения RTC");
            }
            uint64_t t4 = __rdtsc();
            ответа.push_back(t4 - t3);
            uint64_t t5 = __rdtsc();
            do {
                if (ioctl(fd, RTC_RD_TIME, &curr) < 0) {
                    close(fd);
                    throw runtime_error("Ошибка чтения RTC");
                }
                if (++попытки > МАКС_ПОПЫТОК_RTC) {
                    close(fd);
                    throw runtime_error("Превышено число попыток RTC");
                }
                usleep(1000);
            } while (memcmp(&prev, &curr, sizeof(rtc_time)) == 0);
            uint64_t t6 = __rdtsc();
            дельты.push_back(t6 - t5);
            prev = curr;
        }
        close(fd);
        double среднее = accumulate(дельты.begin(), дельты.end(), 0.0) / дельты.size();
        double sq_sum = inner_product(дельты.begin(), дельты.end(), дельты.begin(), 0.0);
        double отклонение = sqrt(sq_sum/дельты.size() - среднее*среднее);
        double a1 = accumulate(инициализации.begin(), инициализации.end(), 0.0) / инициализации.size() / tsc_на_нс;
        double a2 = accumulate(ответа.begin(), ответа.end(), 0.0) / ответа.size() / tsc_на_нс;
        return {
            tsc_в_нс(среднее),
            tsc_в_нс(отклонение),
            1e6,
            a1, a2
        };
    }

    ХарактеристикиТаймера измерить_posix(clockid_t clk) {
        timespec res;
        clock_getres(clk, &res);
        double sys_acc = res.tv_sec*1e9 + res.tv_nsec;
        vector<uint64_t> дельты;
        for (size_t i = 0; i < ИТЕРАЦИЙ; ++i) {
            timespec t1, t2;
            uint64_t s1 = __rdtsc();
            clock_gettime(clk, &t1);
            clock_gettime(clk, &t2);
            uint64_t s2 = __rdtsc();
            дельты.push_back(tsc_в_нс(s2 - s1));
        }
        double среднее = accumulate(дельты.begin(), дельты.end(), 0.0) / дельты.size();
        double sq_sum = inner_product(дельты.begin(), дельты.end(), дельты.begin(), 0.0);
        double отклонение = sqrt(sq_sum/дельты.size() - среднее*среднее);
        return {
            среднее,
            отклонение,
            sys_acc,
            0, 0
        };
    }

    ХарактеристикиТаймера измерить_rdtsc() {
        vector<uint64_t> дельты;
        for (size_t i = 0; i < ИТЕРАЦИЙ; ++i) {
            uint64_t t1 = __rdtsc();
            uint64_t t2 = __rdtsc();
            дельты.push_back(tsc_в_нс(t2 - t1));
        }
        double среднее = accumulate(дельты.begin(), дельты.end(), 0.0) / дельты.size();
        double sq_sum = inner_product(дельты.begin(), дельты.end(), дельты.begin(), 0.0);
        double отклонение = sqrt(sq_sum/дельты.size() - среднее*среднее);
        return {среднее, отклонение, 0, 0, 0};
    }

    void вывести_таблицу() {
        cout << "\nТаблица характеристик таймеров:\n";
        cout << "Тип таймера | Разрешение (нс) | Погрешность (нс) | Сист. точность (нс) | Время инициализации (нс) | Время ответа (нс)\n";
        cout << string(110, '-') << endl;
        cout << "RDTSC         | ";
        auto r = измерить_rdtsc();
        cout << r.разрешение << " | " << r.ошибка_разрешения << " | - | - | -\n";
        cout << "RTC           | ";
        try {
            auto rtc = измерить_rtc();
            cout << rtc.разрешение << " | " << rtc.ошибка_разрешения << " | " << rtc.системная_точность << " | " << rtc.время_инициализации << " | " << rtc.время_ответа << endl;
        } catch (const exception& e) {
            cout << "Ошибка RTC: " << e.what() << endl;
        }
        for (auto clk : поддерживаемые_часы) {
            string name;
            switch (clk) {
                case CLOCK_REALTIME: name = "REALTIME   "; break;
                case CLOCK_MONOTONIC: name = "MONOTONIC  "; break;
                case CLOCK_MONOTONIC_RAW: name = "MONO_RAW   "; break;
                case CLOCK_PROCESS_CPUTIME_ID: name = "PROC_CPU   "; break;
                case CLOCK_THREAD_CPUTIME_ID: name = "THREAD_CPU "; break;
                default: name = "UNKNOWN    "; break;
            }
            cout << name << " | ";
            auto res = измерить_posix(clk);
            cout << res.разрешение << " | " << res.ошибка_разрешения << " | " << res.системная_точность << " | - | -\n";
        }
    }
};

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    cout << "Лабораторная 2. Измерение характеристик таймеров ОС и процессора.\n";
    ИзмерительТаймеров измеритель;
    измеритель.вывести_таблицу();
    return 0;
} 