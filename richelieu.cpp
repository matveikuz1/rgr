#include "richelieu.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <stdexcept>
#include <numeric>
#include <sstream>
#include <vector>
#include <codecvt>
#include <locale>

using namespace std;

// Функция для корректного разделения UTF-8 строки на символы
vector<string> utf8_split(const string& str) {
    vector<string> characters;
    for(size_t i = 0; i < str.size();) {
        unsigned char c = str[i];
        size_t char_len = 1;
        
        // Определяем длину UTF-8 символа по первому байту
        if((c & 0xE0) == 0xC0) char_len = 2;
        else if((c & 0xF0) == 0xE0) char_len = 3;
        else if((c & 0xF8) == 0xF0) char_len = 4;
        
        // Защита от некорректных UTF-8 последовательностей
        if(i + char_len > str.size()) char_len = 1;
        
        characters.push_back(str.substr(i, char_len));
        i += char_len;
    }
    return characters;
}

// Генерация ключа для шифра Ришелье
string generateRichelieuKey(int blockSize) {
    if(blockSize <= 0) throw invalid_argument("Размер блока должен быть положительным");
    
    // Создаем последовательность чисел от 1 до blockSize
    vector<int> permutation(blockSize);
    iota(permutation.begin(), permutation.end(), 1);
    
    // Перемешиваем последовательность для создания случайной перестановки
    random_device rd;
    mt19937 g(rd());
    shuffle(permutation.begin(), permutation.end(), g);
    
    // Формируем строковое представление ключа
    string key;
    for(int num : permutation) {
        key += to_string(num) + " ";
    }
    if(!key.empty()) key.pop_back();
    
    return key;
}

// Разбор строкового ключа в числовой вектор перестановок
vector<int> parseKey(const string& keyStr) {
    vector<int> key;
    istringstream iss(keyStr);
    int num;
    
    // Извлекаем числа из строки ключа
    while(iss >> num) key.push_back(num);
    
    if(key.empty()) throw invalid_argument("Некорректный формат ключа Ришелье");
    
    // Проверяем, что ключ представляет валидную перестановку
    vector<int> sortedKey = key;
    sort(sortedKey.begin(), sortedKey.end());
    for(size_t i = 0; i < sortedKey.size(); ++i) {
        if(sortedKey[i] != i + 1) {
            throw invalid_argument("Некорректный формат ключа Ришелье");
        }
    }
    
    return key;
}

// Функция для дополнения блока символами X до нужного размера
void padBlock(vector<string>& block, size_t blockSize) {
    while (block.size() < blockSize) {
        block.push_back("X"); // Добавляем по одному символу X
    }
}

// Шифрование текста с использованием шифра Ришелье
string richelieuEncrypt(const string& text, const string& keyStr) {
    vector<int> key = parseKey(keyStr);
    vector<string> characters = utf8_split(text);
    string result;
    result.reserve(text.size() + key.size()); // Резервируем память с запасом
    
    // Обрабатываем текст блоками размером с ключ
    for(size_t i = 0; i < characters.size(); i += key.size()) {
        size_t remaining = characters.size() - i;
        size_t block_size = min(key.size(), remaining);
        
        // Создаем блок символов для обработки
        vector<string> block;
        for(size_t j = 0; j < block_size; ++j) {
            block.push_back(characters[i + j]);
        }
        
        // Дополняем блок символами X до размера ключа
        if(block.size() < key.size()) {
            padBlock(block, key.size());
        }
        
        // Применяем перестановку согласно ключу
        for(size_t j = 0; j < key.size(); ++j) {
            size_t pos_in_block = key[j] - 1;
            if(pos_in_block < block.size()) {
                result += block[pos_in_block];
            } else {
                result += block[j]; // fallback, не должно происходить после дополнения
            }
        }
    }
    
    return result;
}

// Дешифрование текста, зашифрованного шифром Ришелье
string richelieuDecrypt(const string& ciphertext, const string& keyStr) {
    vector<int> key = parseKey(keyStr);
    vector<string> characters = utf8_split(ciphertext);
    string result;
    result.reserve(ciphertext.size());
    
    // Создаем обратную перестановку для дешифрования
    vector<int> inverse_key(key.size());
    for(size_t i = 0; i < key.size(); ++i) {
        inverse_key[key[i]-1] = i+1;
    }
    
    // Обрабатываем зашифрованный текст блоками
    for(size_t i = 0; i < characters.size(); i += key.size()) {
        size_t block_size = min(key.size(), characters.size() - i);
        
        // Создаем блок символов для обработки
        vector<string> block;
        for(size_t j = 0; j < block_size; ++j) {
            block.push_back(characters[i + j]);
        }
        
        // Применяем обратную перестановку для восстановления исходного порядка
        for(size_t j = 0; j < key.size(); ++j) {
            size_t pos_in_block = inverse_key[j] - 1;
            if(pos_in_block < block.size()) {
                // Добавляем только оригинальные символы (игнорируем дополнение)
                if (i + key.size() < characters.size() || pos_in_block < block_size) {
                    result += block[pos_in_block];
                }
            }
        }
    }
    
    return result;
}

// Сохранение ключа шифра Ришелье в файл
void saveRichelieuKey(const string& key, const string& filename) {
    ofstream file(filename);
    if(!file) throw runtime_error("Ошибка при записи ключа");
    file << key;
}

// Загрузка ключа шифра Ришелье из файла
string loadRichelieuKey(const string& filename) {
    ifstream file(filename);
    if(!file) throw runtime_error("Ошибка при чтении ключа");
    string key;
    getline(file, key);
    return key;
}
