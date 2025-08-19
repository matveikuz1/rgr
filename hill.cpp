#include "hill.h"
#include <vector>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <numeric>
#include <cctype>
#include <locale>
#include <algorithm>
#include <random>
#include <climits>

using namespace std;
namespace fs = std::filesystem;
// Размер алфавита (все возможные символы char)
const int ALPHABET_SIZE = 256;

// Преобразование символа в индекс
int charToIndex(char c) {
    return static_cast<unsigned char>(c);
}

// Преобразование индекса в символ
char indexToChar(int index) {
    return static_cast<char>(index % ALPHABET_SIZE);
}

// Проверка обратимости матрицы по модулю
bool isMatrixInvertible(const vector<vector<int>>& matrix, int mod) {
    if (matrix.size() != 2 || matrix[0].size() != 2 || matrix[1].size() != 2) {
        return false;
    }
    
    int det = (matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0]) % mod;
    if (det < 0) det += mod;
    return det != 0 && gcd(det, mod) == 1;
}

// Генерация ключа 2x2
vector<vector<int>> generateHillKey(size_t blockSize) {
    vector<vector<int>> key(2, vector<int>(2));
    int mod = ALPHABET_SIZE;
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, mod - 1);

    do {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                key[i][j] = dist(gen);
            }
        }
    } while (!isMatrixInvertible(key, mod));
    
    return key;
}

// Умножение матрицы на вектор
vector<int> matrixMultiply(const vector<vector<int>>& matrix, const vector<int>& vec, int mod) {
    vector<int> result(2);
    for (int i = 0; i < 2; ++i) {
        result[i] = (matrix[i][0] * vec[0] + matrix[i][1] * vec[1]) % mod;
        if (result[i] < 0) result[i] += mod;
    }
    return result;
}

// Обработка текста
string processText(const string& text, const vector<vector<int>>& key, bool decrypt) {
    string result;
    vector<vector<int>> useKey = key;
    int mod = ALPHABET_SIZE;
    
    // Вычисление обратной матрицы для дешифрования
    if (decrypt) {
        int det = (key[0][0] * key[1][1] - key[0][1] * key[1][0]) % mod;
        if (det < 0) det += mod;
        
        int detInv = -1;
        for (int i = 1; i < mod; ++i) {
            if ((det * i) % mod == 1) {
                detInv = i;
                break;
            }
        }
        
        if (detInv == -1) {
            throw runtime_error("Key matrix is not invertible");
        }
        
        useKey = {
            {(key[1][1] * detInv) % mod, (-key[0][1] * detInv) % mod},
            {(-key[1][0] * detInv) % mod, (key[0][0] * detInv) % mod}
        };
        
        // Корректировка отрицательных значений
        for (auto& row : useKey) {
            for (auto& elem : row) {
                if (elem < 0) elem += mod;
            }
        }
    }
    
    // Обработка текста по два символа
    for (size_t i = 0; i < text.size(); ) {
        int idx1 = charToIndex(text[i]);
        size_t j = i + 1;
        
        // Пропускаем необрабатываемые символы
        while (j < text.size() && charToIndex(text[j]) == -1) {
            j++;
        }
        
        if (j < text.size()) {
            int idx2 = charToIndex(text[j]);
            if (idx2 != -1) {
                vector<int> vec = {idx1, idx2};
                vector<int> res = matrixMultiply(useKey, vec, mod);
                
                result += indexToChar(res[0]);
                result += indexToChar(res[1]);
                
                // Добавляем символы между ними
                for (size_t k = i + 1; k < j; ++k) {
                    result += text[k];
                }
                
                i = j + 1;
                continue;
            }
        }
        
        // Если не обрабатывали как пару, добавляем символ как есть
        result += text[i++];
    }
    
    return result;
}

// Шифрование
string hillEncrypt(const string& text, const vector<vector<int>>& key) {
    return processText(text, key, false);
}

// Дешифрование
string hillDecrypt(const string& ciphertext, const vector<vector<int>>& key) {
    return processText(ciphertext, key, true);
}
void hillEncryptFile(const std::string& inputFile, const std::string& outputFile, 
                    const std::vector<std::vector<int>>& key) {
    // Проверка существования входного файла
    if (!fs::exists(inputFile)) {
        throw runtime_error("Ошибка: входной файл не существует: " + inputFile);
    }

    ifstream in(inputFile, ios::binary);
    if (!in) throw runtime_error("Ошибка: не удалось открыть файл: " + inputFile);

    // Создание директорий для выходного файла
    fs::path outPath(outputFile);
    if (outPath.has_parent_path()) {
        fs::create_directories(outPath.parent_path());
    }

    ofstream out(outputFile, ios::binary);
    if (!out) throw runtime_error("Ошибка: не удалось создать файл: " + outputFile);

    string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    string encrypted = hillEncrypt(content, key);
    out << encrypted;
}

void hillDecryptFile(const std::string& inputFile, const std::string& outputFile,
                    const std::vector<std::vector<int>>& key) {
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
    string decrypted = hillDecrypt(content, key);
    out << decrypted;
}

// Сохранение ключа в файл
void saveHillKey(const vector<vector<int>>& key, const string& filename) {
    ofstream file(filename);
    if (!file) throw runtime_error("Cannot open key file");
    file << key[0][0] << " " << key[0][1] << "\n"
         << key[1][0] << " " << key[1][1];
}

// Загрузка ключа из файла
vector<vector<int>> loadHillKey(const string& filename) {
    ifstream file(filename);
    if (!file) throw runtime_error("Cannot open key file");
    
    vector<vector<int>> key(2, vector<int>(2));
    file >> key[0][0] >> key[0][1] >> key[1][0] >> key[1][1];
    
    // Проверка на обратимость
    if (!isMatrixInvertible(key, ALPHABET_SIZE)) {
        throw runtime_error("Loaded key matrix is not invertible");
    }
    
    return key;
}
