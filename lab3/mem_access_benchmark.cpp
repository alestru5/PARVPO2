#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

constexpr int ИСХОДНОЕ_ЗЕРНО = 42;

// Линейный конгруэнтный генератор для быстрой генерации псевдослучайных чисел
class ЛКГ {
    unsigned long множитель, приращение, модуль, состояние;
public:
    ЛКГ(unsigned long зерно, unsigned long множитель = 1664525, unsigned long приращение = 1013904223, unsigned long модуль = 4294967296)
        : множитель(множитель), приращение(приращение), модуль(модуль), состояние(зерно) {}
    unsigned long следующее() {
        состояние = (множитель * состояние + приращение) % модуль;
        return состояние;
    }
};

void измерить_время(int диапазон_k, std::ifstream& файл, std::streamsize размер_файла) {
    const int размер_блока = 8;
    const int число_чтений = 1000000;
    if (размер_файла < диапазон_k) {
        std::cerr << "k превышает размер файла! Пропуск: k = " << диапазон_k << std::endl;
        return;
    }
    ЛКГ генератор(ИСХОДНОЕ_ЗЕРНО);
    unsigned long смещение = генератор.следующее() % (размер_файла - диапазон_k);
    std::cout << "Фиксированное смещение a = " << смещение << std::endl;
    auto старт = std::chrono::high_resolution_clock::now();
    unsigned long контрольная_сумма = 0;
    for (int i = 0; i < число_чтений; ++i) {
        unsigned long адрес = смещение + (генератор.следующее() % диапазон_k);
        if (адрес + размер_блока > размер_файла) continue;
        файл.seekg(адрес, std::ios::beg);
        char буфер[размер_блока];
        файл.read(буфер, размер_блока);
        for (int j = 0; j < размер_блока; ++j) {
            контрольная_сумма += static_cast<unsigned char>(буфер[j]);
        }
    }
    auto конец = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> длительность = конец - старт;
    std::cout << "Время для k = " << диапазон_k << ": " << длительность.count() << " сек, контрольная сумма = " << контрольная_сумма << std::endl;
}

int main() {
    const std::string имя_файла = "/home/youruser/yourvideo.mkv"; // Укажи свой путь к большому файлу
    std::ifstream файл(имя_файла, std::ios::binary | std::ios::ate);
    if (!файл) {
        std::cerr << "Ошибка открытия файла!" << std::endl;
        return 1;
    }
    std::streamsize размер_файла = файл.tellg();
    файл.seekg(0, std::ios::beg);
    std::cout << "Размер файла: " << размер_файла << " байт" << std::endl;
    std::vector<int> значения_k = {24000, 30000, 48000, 80000, 200000, 500000, 1000000, 5000000, 8000000, 20000000, 40000000, 80000000};
    for (int k : значения_k) {
        измерить_время(k, файл, размер_файла);
    }
    файл.close();
    return 0;
} 