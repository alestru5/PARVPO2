#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <string>

using namespace std;

// Генерация случайной строки из латинских букв и цифр
string сгенерировать_строку(int длина) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis('0', '9');
    uniform_int_distribution<> dis_upper('A', 'Z');
    uniform_int_distribution<> dis_lower('a', 'z');
    string результат(длина, ' ');
    for (int i = 0; i < длина; ++i) {
        int выбор = rand() % 3;
        if (выбор == 0) {
            результат[i] = static_cast<char>(dis(gen));
        } else if (выбор == 1) {
            результат[i] = static_cast<char>(dis_upper(gen));
        } else {
            результат[i] = static_cast<char>(dis_lower(gen));
        }
    }
    return результат;
}

chrono::duration<double> записать_в_файл(const string& имя_файла, int строк, int макс_длина) {
    chrono::duration<double> длительность(0);
    auto старт = chrono::high_resolution_clock::now();
    ofstream файл(имя_файла, ios::out);
    if (!файл) {
        cerr << "Ошибка открытия файла!" << endl;
        return длительность;
    }
    auto конец = chrono::high_resolution_clock::now();
    длительность += конец - старт;
    for (int i = 0; i < строк; ++i) {
        int длина = rand() % (макс_длина + 1);
        string строка = сгенерировать_строку(длина);
        старт = chrono::high_resolution_clock::now();
        файл << длина << " " << строка << endl;
        конец = chrono::high_resolution_clock::now();
        длительность += конец - старт;
    }
    старт = chrono::high_resolution_clock::now();
    файл.close();
    конец = chrono::high_resolution_clock::now();
    длительность += конец - старт;
    return длительность;
}

int main() {
    int количество_строк = 3000000;
    int максимальная_длина = 1000;
    auto время = записать_в_файл("данные.txt", количество_строк, максимальная_длина);
    cout << "Время записи в файл: " << время.count() << " секунд." << endl;
    return 0;
} 