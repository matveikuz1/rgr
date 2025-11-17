#include "file.h"
#include <fstream>
#include <stdexcept>
#include <vector>
#include <filesystem>
#include <iostream>
using namespace std;

std::string readFileAsBytes(const std::string& filename) {
    ifstream file(filename, ios::binary | ios::ate);
    if (!file) {
        throw runtime_error("Ошибка: файл не существует или недоступен: " + filename);
    }

    auto size = file.tellg();
    if (size == 0) {
        return ""; // Пустой файл - возвращаем пустую строку
    }
    
    string content(size, '\0');
    file.seekg(0); //создание файла
    file.read(&content[0], size); //чтение файоа в буфер
    
    return content;
}

void writeFileAsBytes(const std::string& filename, const std::string& content) {
    fs::path filepath(filename);
    if (filepath.has_parent_path()) {
        fs::create_directories(filepath.parent_path());
    }

    ofstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Ошибка: не удалось создать файл или директорию: " + filename);
    }
    
    file.write(content.data(), content.size());
    
    if (file.fail()) {
        throw runtime_error("Ошибка записи в файл: " + filename);
    }
}

bool validateFilePath(const string& path, bool checkExists) { //проверка правильного пути
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

bool ensureFileExists(string& filePath) { //Создание файла в нужной директории
    try {
        if (!fs::exists(filePath)) {
            cout << "Файл не существовал. Создаю новый: " << filePath << endl;
            
            fs::path path(filePath);
            if (path.has_parent_path()) {
                fs::create_directories(path.parent_path()); // создание директории
            }
            
            ofstream file(filePath, ios::binary);
            if (!file) {
                cerr << "Ошибка: Не удалось создать файл" << endl;
                return false; //проверка выполнения
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
    while (getline(cin, line) && !line.empty()) { //чтение до ввода пустой строки
        content += line + "\n";
    }
    if (!content.empty()) {
        content.pop_back(); // Удаляем последний \n
    }
    return content;
}
