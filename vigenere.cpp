#include "vigenere.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <stdexcept>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// Шифрование/дешифрование бинарных данных
string vigenereProcess(const string& data, const string& key, bool decrypt) {
    if (key.empty()) throw invalid_argument("Ключ не может быть пустым");
    
    string result;
    result.reserve(data.size()); // резерв памяти
    
    for (size_t i = 0; i < data.size(); ++i) {
        unsigned char dataByte = data[i];
        unsigned char keyByte = key[i % key.size()];
        
        // Побайтовый сдвиг по модулю 256
        int shift = keyByte; 
        if (decrypt) shift = -shift;
        
        int resultByte = (dataByte + shift) % 256; //основа шифрования (формула)
        if (resultByte < 0) resultByte += 256;
        
        result += static_cast<char>(resultByte); // коррекция отрицательных
    }
    return result;
}

// Шифрование
string vigenereEncrypt(const string& data, const string& key) {
    return vigenereProcess(data, key, false);
}

// Дешифрование
string vigenereDecrypt(const string& ciphertext, const string& key) {
    return vigenereProcess(ciphertext, key, true);
}

void vigenereEncryptFile(const std::string& inputFile, const std::string& outputFile,
                        const std::string& key) {
    if (!fs::exists(inputFile)) {
        throw runtime_error("Ошибка: входной файл не существует: " + inputFile);
    }

    ifstream in(inputFile, ios::binary);
    if (!in) throw runtime_error("Ошибка: не удалось открыть файл: " + inputFile);

    fs::path outPath(outputFile);
    if (outPath.has_parent_path()) {
        fs::create_directories(outPath.parent_path());
    }

    ofstream out(outputFile, ios::binary);
    if (!out) throw runtime_error("Ошибка: не удалось создать файл: " + outputFile);

    string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    string encrypted = vigenereEncrypt(content, key);
    out.write(encrypted.data(), encrypted.size());
}

void vigenereDecryptFile(const std::string& inputFile, const std::string& outputFile,
                        const std::string& key) {
    if (!fs::exists(inputFile)) {
        throw runtime_error("Ошибка: входной файл не существует: " + inputFile);
    }

    ifstream in(inputFile, ios::binary);
    if (!in) throw runtime_error("Ошибка: не удалось открыть файл: " + inputFile);

    fs::path outPath(outputFile);
    if (outPath.has_parent_path()) {
        fs::create_directories(outPath.parent_path());
    }

    ofstream out(outputFile, ios::binary);
    if (!out) throw runtime_error("Ошибка: не удалось создать файл: " + outputFile);

    string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    string decrypted = vigenereDecrypt(content, key);
    out.write(decrypted.data(), decrypted.size());
}

// Генерация ключа (случайные байты)
string generateVigenereKey(int length) {
    if (length <= 0) throw invalid_argument("Длина ключа должна быть положительной");
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 255); // Все возможные байты
    
    string key;
    for (int i = 0; i < length; ++i) {
        key += static_cast<char>(dist(gen));
    }
    return key;
}

// Сохранение ключа в бинарный файл
void saveVigenereKey(const string& key, const string& filename) {
    ofstream file(filename, ios::binary);
    if (!file) throw runtime_error("Не удалось открыть файл для записи ключа");
    file.write(key.data(), key.size());
}

// Загрузка ключа из бинарного файла
string loadVigenereKey(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) throw runtime_error("Не удалось открыть файл с ключом");
    
    string key((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return key;
}
