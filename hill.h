#ifndef HILL_H
#define HILL_H

#include <vector>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

// Генерация ключевой матрицы 2x2
__attribute__((visibility("default")))
std::vector<std::vector<int>> generateHillKey(size_t blockSize = 2);

// Шифрование текста
__attribute__((visibility("default")))
std::string hillEncrypt(const std::string& text, const std::vector<std::vector<int>>& key);

// Дешифрование текста
__attribute__((visibility("default")))
std::string hillDecrypt(const std::string& ciphertext, const std::vector<std::vector<int>>& key);

// Сохранение ключа в файл
__attribute__((visibility("default")))
void saveHillKey(const std::vector<std::vector<int>>& key, const std::string& filename);

__attribute__((visibility("default")))
void hillEncryptFile(const std::string& inputFile, const std::string& outputFile,
                    const std::vector<std::vector<int>>& key);

__attribute__((visibility("default")))
void hillDecryptFile(const std::string& inputFile, const std::string& outputFile,
                    const std::vector<std::vector<int>>& key);

// Загрузка ключа из файла
__attribute__((visibility("default")))
std::vector<std::vector<int>> loadHillKey(const std::string& filename);

#ifdef __cplusplus
}
#endif

#endif // HILL_H
