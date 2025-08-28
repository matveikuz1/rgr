#include "vigenere.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <stdexcept>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;
// Шифрование/дешифрование (decrypt = false/true)
// Основная функция обработки текста для шифра Виженера (шифрование/дешифрование)
string vigenereProcess(const string& text, const string& key, bool decrypt) {
    if (key.empty()) throw invalid_argument("Ключ не может быть пустым");
    
    string result;
    for (size_t i = 0; i < text.size(); ++i) {
        char textChar = text[i];
        char keyChar = key[i % key.size()];
        
        // Побайтовый сдвиг по модулю 256
        int shift = static_cast<unsigned char>(keyChar);
        if (decrypt) shift = -shift;
        
        // Выполняем модульное сложение/вычитание
        int encryptedChar = (static_cast<unsigned char>(textChar) + shift) % 256;
        if (encryptedChar < 0) encryptedChar += 256; // Коррекция для дешифрования
        
        result += static_cast<char>(encryptedChar);
    }
    return result;
}

// Шифрование файла с использованием шифра Виженера
void vigenereEncryptFile(const std::string& inputFile, const std::string& outputFile,
                        const std::string& key) {
    if (!fs::exists(inputFile)) {
        throw runtime_error("Ошибка: входной файл не существует: " + inputFile);
    }

    ifstream in(inputFile, ios::binary);
    if (!in) throw runtime_error("Ошибка: не удалось открыть файл: " + inputFile);

    // Создаем директории для выходного файла при необходимости
    fs::path outPath(outputFile);
    if (outPath.has_parent_path()) {
        fs::create_directories(outPath.parent_path());
    }

    ofstream out(outputFile, ios::binary);
    if (!out) throw runtime_error("Ошибка: не удалось создать файл: " + outputFile);

    // Читаем, шифруем и записываем содержимое файла
    string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    string encrypted = vigenereEncrypt(content, key);
    out << encrypted;
}

// Дешифрование файла, зашифрованного шифром Виженера
void vigenereDecryptFile(const std::string& inputFile, const std::string& outputFile,
                        const std::string& key) {
    if (!fs::exists(inputFile)) {
        throw runtime_error("Ошибка: входной файл не существует: " + inputFile);
    }

    ifstream in(inputFile, ios::binary);
    if (!in) throw runtime_error("Ошибка: не удалось открыть файл: " + inputFile);

    // Создаем директории для выходного файла при необходимости
    fs::path outPath(outputFile);
    if (outPath.has_parent_path()) {
        fs::create_directories(outPath.parent_path());
    }

    ofstream out(outputFile, ios::binary);
    if (!out) throw runtime_error("Ошибка: не удалось создать файл: " + outputFile);

    // Читаем, дешифруем и записываем содержимое файла
    string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    string decrypted = vigenereDecrypt(content, key);
    out << decrypted;
}

// Функция шифрования текста шифром Виженера
string vigenereEncrypt(const string& text, const string& key) {
    return vigenereProcess(text, key, false);
}

// Функция дешифрования текста, зашифрованного шифром Виженера
string vigenereDecrypt(const string& ciphertext, const string& key) {
    return vigenereProcess(ciphertext, key, true);
}

// Генерация случайного ключа для шифра Виженера
string generateVigenereKey(int length) {
    if (length <= 0) throw invalid_argument("Длина ключа должна быть положительной");
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(32, 126); // Все возможные байты
    
    // Генерируем ключ из случайных символов
    string key;
    for (int i = 0; i < length; ++i) {
        key += static_cast<char>(dist(gen));
    }
    return key;
}

// Сохранение ключа шифра Виженера в файл (бинарный режим)
void saveVigenereKey(const string& key, const string& filename) {
    ofstream file(filename, ios::binary);
    if (!file) throw runtime_error("Не удалось открыть файл для записи ключа");
    file.write(key.data(), key.size());
}

// Загрузка ключа шифра Виженера из файла
string loadVigenereKey(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) throw runtime_error("Не удалось открыть файл с ключом");
    
    string key((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return key;
}
