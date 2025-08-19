#ifndef VIGENERE_H
#define VIGENERE_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

// Шифрование текста
std::string vigenereEncrypt(const std::string& text, const std::string& key);

// Дешифрование текста
std::string vigenereDecrypt(const std::string& ciphertext, const std::string& key);

// Генерация ключа (случайная строка)
std::string generateVigenereKey(int length);

// Сохранение ключа в файл
void saveVigenereKey(const std::string& key, const std::string& filename);

__attribute__((visibility("default")))
void vigenereEncryptFile(const std::string& inputFile, const std::string& outputFile,
                        const std::string& key);

__attribute__((visibility("default")))
void vigenereDecryptFile(const std::string& inputFile, const std::string& outputFile,
                        const std::string& key);

// Загрузка ключа из файла
std::string loadVigenereKey(const std::string& filename);


#ifdef __cplusplus
}
#endif

#endif // VIGENERE_H
