#include "hill.h"
#include <vector>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <numeric>
#include <random>
#include <climits>

using namespace std;
namespace fs = std::filesystem;

//размер алфавита (все возможные байты)
const int ALPHABET_SIZE = 256;

//преобразование символа в индекс
int charToIndex(char c) {
    return static_cast<unsigned char>(c);
}

// обратное преобразование индекса в символ 
char indexToChar(int index) {
    return static_cast<char>(index % ALPHABET_SIZE);
}

int rashEvklid(int a, int b, int x, int y) {
    int x0 = 1, x1 = 0;
    int y0 = 0, y1 = 1;
    
    while (b != 0) {
        int q = a / b;
        
        int t = x1;
        x1 = x0 - q * x1;
        x0 = t;
        
        t = y1;
        y1 = y0 - q * y1;
        y0 = t;
        
        t = b;
        b = a % b;
        a = t;
    }
    
    x = x0;
    y = y0;
    return a;
}
// Проверка обратимости матрицы по модулю
bool isMatrixInvertible(const vector<vector<int>>& matrix, int mod) {
    if (matrix.size() != 2 || matrix[0].size() != 2 || matrix[1].size() != 2) {
        return false; //проверка что матрица действительно 2 на 2
    }
    
    int det = (matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0]) % mod;
    if (det < 0) det += mod; //приведение к положительному значению

    int x = 0;
    int y = 0;
    int g = rashEvklid(det,mod,x,y);
    return g == 1;

}

// Генерация ключа (матрицы) 2x2
vector<vector<int>> generateHillKey(size_t blockSize) {
    vector<vector<int>> key(2, vector<int>(2));
    int mod = ALPHABET_SIZE;
    
    random_device rd;
    mt19937 gen(rd()); //генератор чисел
    uniform_int_distribution<> dist(0, mod - 1);

    do {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                key[i][j] = dist(gen);
            }
        }
    } while (!isMatrixInvertible(key, mod)); // проверка обратимая ли матрица
    
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

// Обработка бинарных данных
string processBytes(const string& data, const vector<vector<int>>& key, bool decrypt) {
    string result;
    vector<vector<int>> useKey = key;
    int mod = ALPHABET_SIZE;
    
    //вычисление обратной матрицы для дешифрования
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
        //обратная матрица
        useKey = {
            {(key[1][1] * detInv) % mod, (-key[0][1] * detInv) % mod},
            {(-key[1][0] * detInv) % mod, (key[0][0] * detInv) % mod}
        };
        
        //корректировка отрицательных значений (из отриацтельных в положительные
        for (auto& row : useKey) {
            for (auto& elem : row) {
                if (elem < 0) elem += mod;
            }
        }
    }
    
    //обработка данных по два байта
    for (size_t i = 0; i < data.size(); ) {
        if (i + 1 < data.size()) {
            //обрабатываем пару байтов
            int idx1 = charToIndex(data[i]);
            int idx2 = charToIndex(data[i + 1]);
            
            vector<int> vec = {idx1, idx2};
            vector<int> res = matrixMultiply(useKey, vec, mod);
            
            result += indexToChar(res[0]);
            result += indexToChar(res[1]);
            
            i += 2;
        } else {
            // Если остался один байт, добавляем как есть
            result += data[i];
            i += 1;
        }
    }
    
    return result;
}

//шифрование бинарных данных
string hillEncrypt(const string& data, const vector<vector<int>>& key) {
    return processBytes(data, key, false);
}

// дешифрование бинарных данных
string hillDecrypt(const string& ciphertext, const vector<vector<int>>& key) {
    return processBytes(ciphertext, key, true);
}

void hillEncryptFile(const std::string& inputFile, const std::string& outputFile, 
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

    // Читаем как бинарные данные
    string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    string encrypted = hillEncrypt(content, key);
    out.write(encrypted.data(), encrypted.size());
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
    out.write(decrypted.data(), decrypted.size());
}

// сохранение ключа в бинарный файл
void saveHillKey(const vector<vector<int>>& key, const string& filename) {
    ofstream file(filename, ios::binary);
    if (!file) throw runtime_error("Cannot open key file");
    
    for (const auto& row : key) {
        for (int val : row) {
            file.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }
    }
}

// Загрузка ключа из бинарного файла
vector<vector<int>> loadHillKey(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) throw runtime_error("Cannot open key file");
    
    vector<vector<int>> key(2, vector<int>(2));
    for (auto& row : key) {
        for (int& val : row) {
            file.read(reinterpret_cast<char*>(&val), sizeof(val));
        }
    }
    
    // Проверка на обратимость
    if (!isMatrixInvertible(key, ALPHABET_SIZE)) {
        throw runtime_error("Loaded key matrix is not invertible");
    }
    
    return key;
}
