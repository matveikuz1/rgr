#ifndef RICHELIEU_H
#define RICHELIEU_H

#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

// Шифрование текста (работает с любыми char 256)
std::string richelieuEncrypt(const std::string& text, const std::string& key);

// Дешифрование текста (работает с любыми char 256)
std::string richelieuDecrypt(const std::string& ciphertext, const std::string& key);

// Генерация ключа (случайная перестановка для blockSize символов)
std::string generateRichelieuKey(int blockSize);

// Сохранение ключа в файл
void saveRichelieuKey(const std::string& key, const std::string& filename);

// Загрузка ключа из файла
std::string loadRichelieuKey(const std::string& filename);

#ifdef __cplusplus
}
#endif

#endif // RICHELIEU_H
