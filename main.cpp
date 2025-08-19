#include <iostream>
#include <stdexcept>
#include <limits>
#include <string>
#include <cstdlib>
#include <dlfcn.h>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "file.h"
#include <fstream>
#include <locale.h>
#include <vector>
#include <functional>
#include <optional>

using namespace std;
namespace fs = std::filesystem;

// Определения типов для функций из библиотек
typedef string (*hillEncryptFunc)(const string&, const vector<vector<int>>&);
typedef string (*hillDecryptFunc)(const string&, const vector<vector<int>>&);
typedef vector<vector<int>> (*generateHillKeyFunc)(size_t);
typedef void (*saveHillKeyFunc)(const vector<vector<int>>&, const string&);
typedef vector<vector<int>> (*loadHillKeyFunc)(const string&);

typedef string (*richelieuEncryptFunc)(const string&, const string&);
typedef string (*richelieuDecryptFunc)(const string&, const string&);
typedef string (*generateRichelieuKeyFunc)(int);
typedef void (*saveRichelieuKeyFunc)(const string&, const string&);
typedef string (*loadRichelieuKeyFunc)(const string&);

typedef string (*vigenereEncryptFunc)(const string&, const string&);
typedef string (*vigenereDecryptFunc)(const string&, const string&);
typedef string (*generateVigenereKeyFunc)(int);
typedef void (*saveVigenereKeyFunc)(const string&, const string&);
typedef string (*loadVigenereKeyFunc)(const string&);

enum class Cipher {
    HILL,
    RICHELIEU,
    VIGENERE,
    EXIT,
    UNKNOWN
};

enum class DataSource {
    CONSOLE,
    FILE
};

optional<DataSource> selectDataSource() {
    while (true) {
        cout << "\nВыберите источник данных:\n";
        cout << "1. Ввести текст с консоли\n";
        cout << "2. Загрузить из файла\n";
        cout << "3. Назад\n";
        cout << "Выбор: ";
        
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Ошибка: Пожалуйста, введите число.\n";
            continue;
        }
        cin.ignore();
        
        if (choice == 1) return DataSource::CONSOLE;
        if (choice == 2) return DataSource::FILE;
        if (choice == 3) return nullopt;
        
        cout << "Ошибка: Неверный выбор, попробуйте снова.\n";
    }
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");

    // Загрузка библиотек
    void* hillLib = dlopen("./libhill.so", RTLD_LAZY);
    void* richelieuLib = dlopen("./librichelieu.so", RTLD_LAZY);
    void* vigenereLib = dlopen("./libvigenere.so", RTLD_LAZY);

    // Инициализация указателей на функции
    hillEncryptFunc hillEncrypt = nullptr;
    hillDecryptFunc hillDecrypt = nullptr;
    generateHillKeyFunc generateHillKey = nullptr;
    saveHillKeyFunc saveHillKey = nullptr;
    loadHillKeyFunc loadHillKey = nullptr;

    richelieuEncryptFunc richelieuEncrypt = nullptr;
    richelieuDecryptFunc richelieuDecrypt = nullptr;
    generateRichelieuKeyFunc generateRichelieuKey = nullptr;
    saveRichelieuKeyFunc saveRichelieuKey = nullptr;
    loadRichelieuKeyFunc loadRichelieuKey = nullptr;

    vigenereEncryptFunc vigenereEncrypt = nullptr;
    vigenereDecryptFunc vigenereDecrypt = nullptr;
    generateVigenereKeyFunc generateVigenereKey = nullptr;
    saveVigenereKeyFunc saveVigenereKey = nullptr;
    loadVigenereKeyFunc loadVigenereKey = nullptr;

    // Загрузка функций Hill
    if (hillLib) {
        hillEncrypt = (hillEncryptFunc)dlsym(hillLib, "hillEncrypt");
        hillDecrypt = (hillDecryptFunc)dlsym(hillLib, "hillDecrypt");
        generateHillKey = (generateHillKeyFunc)dlsym(hillLib, "generateHillKey");
        saveHillKey = (saveHillKeyFunc)dlsym(hillLib, "saveHillKey");
        loadHillKey = (loadHillKeyFunc)dlsym(hillLib, "loadHillKey");

        if (!hillEncrypt || !hillDecrypt || !generateHillKey || !saveHillKey || !loadHillKey) {
            cerr << "Ошибка: При загрузке функций Hill: " << dlerror() << endl;
            dlclose(hillLib);
            hillLib = nullptr;
        }
    }

    // Загрузка функций Richelieu
    if (richelieuLib) {
        richelieuEncrypt = (richelieuEncryptFunc)dlsym(richelieuLib, "richelieuEncrypt");
        richelieuDecrypt = (richelieuDecryptFunc)dlsym(richelieuLib, "richelieuDecrypt");
        generateRichelieuKey = (generateRichelieuKeyFunc)dlsym(richelieuLib, "generateRichelieuKey");
        saveRichelieuKey = (saveRichelieuKeyFunc)dlsym(richelieuLib, "saveRichelieuKey");
        loadRichelieuKey = (loadRichelieuKeyFunc)dlsym(richelieuLib, "loadRichelieuKey");

        if (!richelieuEncrypt || !richelieuDecrypt || !generateRichelieuKey || !saveRichelieuKey || !loadRichelieuKey) {
            cerr << "Ошибка: При загрузке функций Richelieu: " << dlerror() << endl;
            dlclose(richelieuLib);
            richelieuLib = nullptr;
        }
    }

    // Загрузка функций Vigenere
    if (vigenereLib) {
        vigenereEncrypt = (vigenereEncryptFunc)dlsym(vigenereLib, "vigenereEncrypt");
        vigenereDecrypt = (vigenereDecryptFunc)dlsym(vigenereLib, "vigenereDecrypt");
        generateVigenereKey = (generateVigenereKeyFunc)dlsym(vigenereLib, "generateVigenereKey");
        saveVigenereKey = (saveVigenereKeyFunc)dlsym(vigenereLib, "saveVigenereKey");
        loadVigenereKey = (loadVigenereKeyFunc)dlsym(vigenereLib, "loadVigenereKey");

        if (!vigenereEncrypt || !vigenereDecrypt || !generateVigenereKey || !saveVigenereKey || !loadVigenereKey) {
            cerr << "Ошибка: При загрузке функций Vigenere: " << dlerror() << endl;
            dlclose(vigenereLib);
            vigenereLib = nullptr;
        }
    }

    while (true) {
        try {
            cout << "\n==МЕНЮ ШИФРОВАНИЯ/ДЕШИФРОВАНИЯ==\n";
            cout << "1. Шифр Хилла" << (hillLib ? "" : " (недоступно)") << "\n";
            cout << "2. Шифр Ришелье" << (richelieuLib ? "" : " (недоступно)") << "\n";
            cout << "3. Шифр Виженера" << (vigenereLib ? "" : " (недоступно)") << "\n";
            cout << "4. Выход\n";
            cout << "Выберите алгоритм (либо выход): ";

            int choice;
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Ошибка: Пожалуйста, введите число.\n";
                continue;
            }
            cin.ignore();

            Cipher cipherChoice;
            switch (choice) {
                case 1: cipherChoice = Cipher::HILL; break;
                case 2: cipherChoice = Cipher::RICHELIEU; break;
                case 3: cipherChoice = Cipher::VIGENERE; break;
                case 4: cipherChoice = Cipher::EXIT; break;
                default: cipherChoice = Cipher::UNKNOWN;
            }

            if (cipherChoice == Cipher::EXIT) break;
            if (cipherChoice == Cipher::UNKNOWN) {
                cout << "Ошибка: Неизвестный алгоритм. Попробуйте снова.\n";
                continue;
            }

            while (true) {
                try {
                    switch (cipherChoice) {
                        case Cipher::HILL: {
                            if (!hillLib) {
                                cout << "Ошибка: Библиотека Hill не загружена!\n";
                                break;
                            }
                            
                            while (true) {
                                try {
                                    cout << "\n==Шифр Хилла==\n";
                                    cout << "1. Шифрование\n2. Дешифрование\n3. Назад\n";
                                    cout << "Выберите действие: ";
                                    
                                    int action;
                                    if (!(cin >> action)) {
                                        cin.clear();
                                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                        cout << "Ошибка: Пожалуйста, введите число.\n";
                                        continue;
                                    }
                                    cin.ignore();

                                    if (action == 3) break;
                                    if (action != 1 && action != 2) {
                                        cout << "Ошибка: Неизвестная команда. Попробуйте снова.\n";
                                        continue;
                                    }

                                    string keyFile;
                                    bool isEncrypt = (action == 1);
                                    
                                    if (isEncrypt) {
                                        // Шифрование - генерируем ключ
                                        bool keyFileValid = false;
                                        while (!keyFileValid) {
                                            cout << "Введите путь для сохранения ключа: ";
                                            getline(cin, keyFile);
                                            
                                            if (keyFile.empty()) {
                                                cout << "Ошибка: Имя файла не может быть пустым\n";
                                                continue;
                                            }
                                            
                                            if (!ensureFileExists(keyFile)) {
                                                cout << "Ошибка: Не удалось создать файл. Попробуйте еще раз.\n";
                                                continue;
                                            }
                                            
                                            keyFileValid = true;
                                        }

                                        try {
                                            vector<vector<int>> key = generateHillKey(2);
                                            saveHillKey(key, keyFile);
                                            cout << "Ключ сгенерирован и сохранен в " << keyFile << endl;
                                        } catch (const exception& e) {
                                            cerr << "Ошибка: " << e.what() << endl;
                                            continue;
                                        }
                                    } else {
                                        // Дешифрование - загружаем ключ
                                        bool keyFileValid = false;
                                        while (!keyFileValid) {
                                            cout << "Введите путь к файлу с ключом: ";
                                            getline(cin, keyFile);
                                            
                                            if (!validateFilePath(keyFile)) {
                                                cout << "Ошибка: Неверный путь к файлу. Попробуйте еще раз.\n";
                                                continue;
                                            }
                                            
                                            keyFileValid = true;
                                        }
                                    }

                                    // Выбор источника данных
                                    auto source = selectDataSource();
                                    if (!source) continue;

                                    string content;
                                    if (*source == DataSource::CONSOLE) {
                                        content = readFromConsole();
                                    } else {
                                        string inputFile;
                                        bool inputFileValid = false;
                                        while (!inputFileValid) {
                                            cout << "Введите путь к входному файлу: ";
                                            getline(cin, inputFile);
                                            
                                            if (!validateFilePath(inputFile)) {
                                                cout << "Ошибка: Неверный путь к файлу. Попробуйте еще раз.\n";
                                                continue;
                                            }
                                            
                                            inputFileValid = true;
                                        }
                                        content = readFileAsString(inputFile);
                                    }

                                    string outputFile;
                                    bool outputFileValid = false;
                                    while (!outputFileValid) {
                                        cout << "Введите путь для сохранения результата: ";
                                        getline(cin, outputFile);
                                        
                                        if (outputFile.empty()) {
                                            cout << "Ошибка: Имя файла не может быть пустым\n";
                                            continue;
                                        }
                                        
                                        if (!ensureFileExists(outputFile)) {
                                            cout << "Ошибка: Не удалось создать файл. Попробуйте еще раз.\n";
                                            continue;
                                        }
                                        
                                        outputFileValid = true;
                                    }

                                    try {
                                        string result;
                                        if (isEncrypt) {
                                            vector<vector<int>> key = loadHillKey(keyFile);
                                            result = hillEncrypt(content, key);
                                            cout << "Зашифрованный текст:\n" << result << "\n";
                                        } else {
                                            vector<vector<int>> key = loadHillKey(keyFile);
                                            result = hillDecrypt(content, key);
                                            cout << "Расшифрованный текст:\n" << result << "\n";
                                        }
                                        writeFile(outputFile, result);
                                        cout << "Результат сохранен в " << outputFile << endl;
                                    } catch (const exception& e) {
                                        cerr << "Ошибка: " << e.what() << endl;
                                    }
                                } catch (const exception& e) {
                                    cerr << "Ошибка: " << e.what() << endl;
                                    cin.clear();
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                }
                            }
                            break;
                        }
                        case Cipher::RICHELIEU: {
                            if (!richelieuLib) {
                                cout << "Ошибка: Библиотека Richelieu не загружена!\n";
                                break;
                            }
                            
                            while (true) {
                                try {
                                    cout << "\n==Шифр Ришелье==\n";
                                    cout << "1. Шифрование\n2. Дешифрование\n3. Назад\n";
                                    cout << "Выберите действие: ";
                                    
                                    int action;
                                    if (!(cin >> action)) {
                                        cin.clear();
                                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                        cout << "Ошибка: Пожалуйста, введите число.\n";
                                        continue;
                                    }
                                    cin.ignore();

                                    if (action == 3) break;
                                    if (action != 1 && action != 2) {
                                        cout << "Ошибка: Неизвестная команда. Попробуйте снова.\n";
                                        continue;
                                    }

                                    string keyFile;
                                    bool isEncrypt = (action == 1);
                                    
                                    if (isEncrypt) {
                                        // Шифрование - генерируем ключ
                                        int blockSize;
                                        while (true) {
                                            cout << "Введите размер блока для ключа: ";
                                            if (!(cin >> blockSize)) {
                                                cin.clear();
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                                cout << "Ошибка: Пожалуйста, введите число.\n";
                                                continue;
                                            }
                                            cin.ignore();
                                            if (blockSize <= 0) {
                                                cout << "Ошибка: Размер блока должен быть положительным\n";
                                                continue;
                                            }
                                            break;
                                        }

                                        bool keyFileValid = false;
                                        while (!keyFileValid) {
                                            cout << "Введите путь для сохранения ключа: ";
                                            getline(cin, keyFile);
                                            
                                            if (keyFile.empty()) {
                                                cout << "Ошибка: Имя файла не может быть пустым\n";
                                                continue;
                                            }
                                            
                                            if (!ensureFileExists(keyFile)) {
                                                cout << "Ошибка: Не удалось создать файл. Попробуйте еще раз.\n";
                                                continue;
                                            }
                                            
                                            keyFileValid = true;
                                        }

                                        try {
                                            string key = generateRichelieuKey(blockSize);
                                            saveRichelieuKey(key, keyFile);
                                            cout << "Ключ сгенерирован и сохранен в " << keyFile << endl;
                                        } catch (const exception& e) {
                                            cerr << "Ошибка: " << e.what() << endl;
                                            continue;
                                        }
                                    } else {
                                        // Дешифрование - загружаем ключ
                                        bool keyFileValid = false;
                                        while (!keyFileValid) {
                                            cout << "Введите путь к файлу с ключом: ";
                                            getline(cin, keyFile);
                                            
                                            if (!validateFilePath(keyFile)) {
                                                cout << "Ошибка: Неверный путь к файлу. Попробуйте еще раз.\n";
                                                continue;
                                            }
                                            
                                            keyFileValid = true;
                                        }
                                    }

                                    // Выбор источника данных
                                    auto source = selectDataSource();
                                    if (!source) continue;

                                    string content;
                                    if (*source == DataSource::CONSOLE) {
                                        content = readFromConsole();
                                    } else {
                                        string inputFile;
                                        bool inputFileValid = false;
                                        while (!inputFileValid) {
                                            cout << "Введите путь к входному файлу: ";
                                            getline(cin, inputFile);
                                            
                                            if (!validateFilePath(inputFile)) {
                                                cout << "Ошибка: Неверный путь к файлу. Попробуйте еще раз.\n";
                                                continue;
                                            }
                                            
                                            inputFileValid = true;
                                        }
                                        content = readFileAsString(inputFile);
                                    }

                                    string outputFile;
                                    bool outputFileValid = false;
                                    while (!outputFileValid) {
                                        cout << "Введите путь для сохранения результата: ";
                                        getline(cin, outputFile);
                                        
                                        if (outputFile.empty()) {
                                            cout << "Ошибка: Имя файла не может быть пустым\n";
                                            continue;
                                        }
                                        
                                        if (!ensureFileExists(outputFile)) {
                                            cout << "Ошибка: Не удалось создать файл. Попробуйте еще раз.\n";
                                            continue;
                                        }
                                        
                                        outputFileValid = true;
                                    }

                                    try {
                                        string result;
                                        if (isEncrypt) {
                                            string key = loadRichelieuKey(keyFile);
                                            result = richelieuEncrypt(content, key);
                                            cout << "Зашифрованный текст:\n" << result << "\n";
                                        } else {
                                            string key = loadRichelieuKey(keyFile);
                                            result = richelieuDecrypt(content, key);
                                            cout << "Расшифрованный текст:\n" << result << "\n";
                                        }
                                        writeFile(outputFile, result);
                                        cout << "Результат сохранен в " << outputFile << endl;
                                    } catch (const exception& e) {
                                        cerr << "Ошибка: " << e.what() << endl;
                                    }
                                } catch (const exception& e) {
                                    cerr << "Ошибка: " << e.what() << endl;
                                    cin.clear();
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                }
                            }
                            break;
                        }
                        case Cipher::VIGENERE: {
                            if (!vigenereLib) {
                                cout << "Ошибка: Библиотека Vigenere не загружена!\n";
                                break;
                            }
                            
                            while (true) {
                                try {
                                    cout << "\n==Шифр Виженера==\n";
                                    cout << "1. Шифрование\n2. Дешифрование\n3. Назад\n";
                                    cout << "Выберите действие: ";
                                    
                                    int action;
                                    if (!(cin >> action)) {
                                        cin.clear();
                                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                        cout << "Ошибка: Пожалуйста, введите число.\n";
                                        continue;
                                    }
                                    cin.ignore();

                                    if (action == 3) break;
                                    if (action != 1 && action != 2) {
                                        cout << "Ошибка: Неизвестная команда. Попробуйте снова.\n";
                                        continue;
                                    }

                                    string keyFile;
                                    bool isEncrypt = (action == 1);
                                    
                                    if (isEncrypt) {
                                        // Шифрование - генерируем ключ
                                        int length;
                                        while (true) {
                                            cout << "Введите длину ключа: ";
                                            if (!(cin >> length)) {
                                                cin.clear();
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                                cout << "Ошибка: Пожалуйста, введите число.\n";
                                                continue;
                                            }
                                            cin.ignore();
                                            if (length <= 0) {
                                                cout << "Ошибка: Длина ключа должна быть положительной\n";
                                                continue;
                                            }
                                            break;
                                        }

                                        bool keyFileValid = false;
                                        while (!keyFileValid) {
                                            cout << "Введите путь для сохранения ключа: ";
                                            getline(cin, keyFile);
                                            
                                            if (keyFile.empty()) {
                                                cout << "Ошибка: Имя файла не может быть пустым\n";
                                                continue;
                                            }
                                            
                                            if (!ensureFileExists(keyFile)) {
                                                cout << "Ошибка: Не удалось создать файл. Попробуйте еще раз.\n";
                                                continue;
                                            }
                                            
                                            keyFileValid = true;
                                        }

                                        try {
                                            string key = generateVigenereKey(length);
                                            saveVigenereKey(key, keyFile);
                                            cout << "Ключ сгенерирован и сохранен в " << keyFile << endl;
                                        } catch (const exception& e) {
                                            cerr << "Ошибка: " << e.what() << endl;
                                            continue;
                                        }
                                    } else {
                                        // Дешифрование - загружаем ключ
                                        bool keyFileValid = false;
                                        while (!keyFileValid) {
                                            cout << "Введите путь к файлу с ключом: ";
                                            getline(cin, keyFile);
                                            
                                            if (!validateFilePath(keyFile)) {
                                                cout << "Ошибка: Неверный путь к файлу. Попробуйте еще раз.\n";
                                                continue;
                                            }
                                            
                                            keyFileValid = true;
                                        }
                                    }

                                    // Выбор источника данных
                                    auto source = selectDataSource();
                                    if (!source) continue;

                                    string content;
                                    if (*source == DataSource::CONSOLE) {
                                        content = readFromConsole();
                                    } else {
                                        string inputFile;
                                        bool inputFileValid = false;
                                        while (!inputFileValid) {
                                            cout << "Введите путь к входному файлу: ";
                                            getline(cin, inputFile);
                                            
                                            if (!validateFilePath(inputFile)) {
                                                cout << "Ошибка: Неверный путь к файлу. Попробуйте еще раз.\n";
                                                continue;
                                            }
                                            
                                            inputFileValid = true;
                                        }
                                        content = readFileAsString(inputFile);
                                    }

                                    string outputFile;
                                    bool outputFileValid = false;
                                    while (!outputFileValid) {
                                        cout << "Введите путь для сохранения результата: ";
                                        getline(cin, outputFile);
                                        
                                        if (outputFile.empty()) {
                                            cout << "Ошибка: Имя файла не может быть пустым\n";
                                            continue;
                                        }
                                        
                                        if (!ensureFileExists(outputFile)) {
                                            cout << "Ошибка: Не удалось создать файл. Попробуйте еще раз.\n";
                                            continue;
                                        }
                                        
                                        outputFileValid = true;
                                    }

                                    try {
                                        string result;
                                        if (isEncrypt) {
                                            string key = loadVigenereKey(keyFile);
                                            result = vigenereEncrypt(content, key);
                                            cout << "Зашифрованный текст:\n" << result << "\n";
                                        } else {
                                            string key = loadVigenereKey(keyFile);
                                            result = vigenereDecrypt(content, key);
                                            cout << "Расшифрованный текст:\n" << result << "\n";
                                        }
                                        writeFile(outputFile, result);
                                        cout << "Результат сохранен в " << outputFile << endl;
                                    } catch (const exception& e) {
                                        cerr << "Ошибка: " << e.what() << endl;
                                    }
                                } catch (const exception& e) {
                                    cerr << "Ошибка: " << e.what() << endl;
                                    cin.clear();
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    break; // Выход из внутреннего цикла при выборе "Назад"
                } catch (const exception& e) {
                    cerr << "Ошибка: " << e.what() << endl;
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            }
        } catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    if (hillLib) dlclose(hillLib);
    if (richelieuLib) dlclose(richelieuLib);
    if (vigenereLib) dlclose(vigenereLib);

    return 0;
}
