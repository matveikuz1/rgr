#include "file.h"
#include <fstream>
#include <stdexcept>
#include <locale>
#include <vector>
#include <filesystem>
#include <iostream>
using namespace std;

std::string readFileAsString(const std::string& filename) {
    ifstream file(filename, ios::binary | ios::ate);
    if (!file) {
        throw runtime_error("Ошибка: файл не существует или недоступен: " + filename);
    }

    auto size = file.tellg();
    if (size == 0) {
        throw runtime_error("Ошибка: файл пуст: " + filename);
    }
    file.seekg(0, ios::beg);

    file.imbue(locale("ru_RU.UTF-8"));
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    
    return content;
}

void writeFile(const std::string& filename, const std::string& content) {
    fs::path filepath(filename);
    if (filepath.has_parent_path()) {
        fs::create_directories(filepath.parent_path());
    }

    ofstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Ошибка: не удалось создать файл или директорию: " + filename);
    }
    
    file.imbue(locale("ru_RU.UTF-8"));
    file << content;
    
    if (file.fail()) {
        throw runtime_error("Ошибка записи в файл: " + filename);
    }
}

bool validateFilePath(const string& path, bool checkExists) {
    if (path.empty()) {
        cerr << "Ошибка: Путь не может быть пустым" << endl;
        return false;
    }

    if (checkExists && !fs::exists(path)) {
        cerr << "Ошибка: Файл или директория не существует: " << path << endl;
        return false;
    }

    return true;
}

bool ensureFileExists(string& filePath) {
    try {
        if (!fs::exists(filePath)) {
            cout << "Файл не существовал. Создаю новый: " << filePath << endl;
            
            fs::path path(filePath);
            if (path.has_parent_path()) {
                fs::create_directories(path.parent_path());
            }
            
            ofstream file(filePath);
            if (!file) {
                cerr << "Ошибка: Не удалось создать файл" << endl;
                return false;
            }
        }
        return true;
    } catch (const exception& e) {
        cerr << "Ошибка: При работе с файлом: " << e.what() << endl;
        return false;
    }
}

string readFromConsole() {
    cout << "Введите текст (завершите пустой строкой):\n";
    string content;
    string line;
    while (getline(cin, line) && !line.empty()) {
        content += line + "\n";
    }
    if (!content.empty()) {
        content.pop_back(); // Удаляем последний \n
    }
    return content;
}
