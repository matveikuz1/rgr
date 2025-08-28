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

//256 для полного охвата ASCII
const int ALPHABET_SIZE = 256;

// Преобразование символа в числовой индекс 
int charToIndex(char c) {
    return static_cast<unsigned char>(c); // Используем unsigned char для корректного преобразования
}

// Преобразование числового индекса обратно в символ
char indexToChar(int index) {
    return static_cast<char>(index % ALPHABET_SIZE); // Обеспечиваем корректное преобразование с учетом модуля
}

// Проверка обратимости матрицы по модулю ALPHABET_SIZE
bool isMatrixInvertible(const vector<vector<int>>& matrix, int mod) {
    // Проверяем, что матрица 2x2
    if (matrix.size() != 2 || matrix[0].size() != 2 || matrix[1].size() != 2) {
        return false;
    }
    
    // Вычисляем детерминант матрицы по модулю
    int det = (matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0]) % mod;
    if (det < 0) det += mod; // Корректируем отрицательные значения
    
    // Матрица обратима, если детерминант не равен 0 и взаимно прост с модулем
    return det != 0 && gcd(det, mod) == 1;
}

// Генерация обратимой матрицы ключа 2x2
vector<vector<int>> generateHillKey(size_t blockSize) {
    vector<vector<int>> key(2, vector<int>(2)); // Создаем матрицу 2x2
    int mod = ALPHABET_SIZE;
    
    // Инициализация генератора случайных чисел
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, mod - 1);

    // Генерируем обратиму матрицу
    do {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                key[i][j] = dist(gen); // Заполняем случайными значениями
            }
        }
    } while (!isMatrixInvertible(key, mod)); 
    return key;
}

// Умножение матрицы на вектор с учетом модульной арифметики
vector<int> matrixMultiply(const vector<vector<int>>& matrix, const vector<int>& vec, int mod) {
    vector<int> result(2);
    for (int i = 0; i < 2; ++i) {
        // Вычисляем скалярное произведение строки матрицы на вектор
        result[i] = (matrix[i][0] * vec[0] + matrix[i][1] * vec[1]) % mod;
        if (result[i] < 0) result[i] += mod; 
    }
    return result;
}

// Основная функция обработки текста (шифрование/дешифрование)
string processText(const string& text, const vector<vector<int>>& key, bool decrypt) {
    string result;
    vector<vector<int>> useKey = key; // Ключ, который будет использоваться
    int mod = ALPHABET_SIZE;
    
    // Если режим дешифрования, вычисляем обратную матрицу
    if (decrypt) {
        // Вычисляем детерминант исходной матрицы
        int det = (key[0][0] * key[1][1] - key[0][1] * key[1][0]) % mod;
        if (det < 0) det += mod;
        
        // Ищем обратный элемент детерминанта по модулю
        int detInv = -1;
        for (int i = 1; i < mod; ++i) {
            if ((det * i) % mod == 1) {
                detInv = i;
                break;
            }
        }
        
        // Если обратный элемент не найден - матрица не обратима
        if (detInv == -1) {
            throw runtime_error("Key matrix is not invertible");
        }
        
        // Вычисляем обратную матрицу по формуле для матрицы 2x2
        useKey = {
            {(key[1][1] * detInv) % mod, (-key[0][1] * detInv) % mod},
            {(-key[1][0] * detInv) % mod, (key[0][0] * detInv) % mod}
        };
        
        // Корректируем отрицательные значения в обратной матрице
        for (auto& row : useKey) {
            for (auto& elem : row) {
                if (elem < 0) elem += mod;
            }
        }
    }
    
    // Обрабатываем текст поблочно (по 2 символа)
    for (size_t i = 0; i < text.size(); ) {
        int idx1 = charToIndex(text[i]); // Преобразуем первый символ в индекс
        size_t j = i + 1;
        
        // Пропускаем необрабатываемые символы между i и j
        while (j < text.size() && charToIndex(text[j]) == -1) {
            j++;
        }
        
        // Если нашли пару символов для обработки
        if (j < text.size()) {
            int idx2 = charToIndex(text[j]); // Преобразуем второй символ в индекс
            if (idx2 != -1) {
                // Создаем вектор из двух символов
                vector<int> vec = {idx1, idx2};
                // Умножаем матрицу на вектор
                vector<int> res = matrixMultiply(useKey, vec, mod);
                
                // Добавляем результат в выходную строку
                result += indexToChar(res[0]);
                result += indexToChar(res[1]);
                
                // Сохраняем символы между обрабатываемой парой (пробелы, знаки препинания)
                for (size_t k = i + 1; k < j; ++k) {
                    result += text[k];
                }
                
                // Переходим к следующей паре
                i = j + 1;
                continue;
            }
        }
        
        // Если не удалось обработать пару, добавляем символ как есть
        result += text[i++];
    }
    
    return result;
}

// Функция шифрования текста
string hillEncrypt(const string& text, const vector<vector<int>>& key) {
    return processText(text, key, false); 
}

// Функция дешифрования текста
string hillDecrypt(const string& ciphertext, const vector<vector<int>>& key) {
    return processText(ciphertext, key, true); 
}

// Шифрование файла
void hillEncryptFile(const std::string& inputFile, const std::string& outputFile, 
                    const std::vector<std::vector<int>>& key) {
    // Проверка существования входного файла
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

    string encrypted = hillEncrypt(content, key);
    // Записываем результат в выходной файл
    out << encrypted;
}

// Дешифрование файла
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
    // Дешифруем содержимое
    string decrypted = hillDecrypt(content, key);

    out << decrypted;
}

// Сохранение ключевой матрицы в файл
void saveHillKey(const vector<vector<int>>& key, const string& filename) {
    ofstream file(filename);
    if (!file) throw runtime_error("Cannot open key file");
    // Сохраняем матрицу в виде: a11 a12 a21 a22
    file << key[0][0] << " " << key[0][1] << "\n"
         << key[1][0] << " " << key[1][1];
}

// Загрузка ключевой матрицы из файла
vector<vector<int>> loadHillKey(const string& filename) {
    ifstream file(filename);
    if (!file) throw runtime_error("Cannot open key file");
    
    // Создаем матрицу 2x2
    vector<vector<int>> key(2, vector<int>(2));

    file >> key[0][0] >> key[0][1] >> key[1][0] >> key[1][1];
    
    // Проверяем, что загруженная матрица обратима
    if (!isMatrixInvertible(key, ALPHABET_SIZE)) {
        throw runtime_error("Loaded key matrix is not invertible");
    }
    
    return key;
}
