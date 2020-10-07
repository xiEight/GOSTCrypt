#ifndef GOST_H
#define GOST_H

#include <cstdint>
#include <string>
#include <fstream>
#include <functional>
#include <QDebug>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <bitset>
#include <cmath>

#define SIMPLECRYPT 0
#define SIMPLEENCRYPT 1


class gost
{
public:
    gost();
    ~gost();
    //Инициализация ключа из файла
    void set_key(std::string filepath);
    //Начало шифрования
    //TODO: проверить, инициироан ли путь!
    void start(std::string inputFile, std::string outputFile, ushort mode = SIMPLECRYPT);

private:

    //Замена типа
    using byte = std::uint8_t;

    //Таблица замены
    std::uint8_t table[8][16] = {{4,10,9,2,13,8,0,14,6,11,1,12,7,15,5,3}, //Таблица S
                        {14,11,4,12,6,13,15,10,2,3,8,1,0,7,5,9},
                        {5,8,1,13,10,3,4,2,14,15,12,7,6,0,9,11},
                        {7,13,10,1,0,8,9,15,14,4,6,12,11,2,5,3},
                        {6,12,7,1,5,15,13,8,4,10,9,14,0,3,11,2},
                        {4,11,10,0,7,2,1,13,3,6,8,5,9,12,15,14},
                        {13,11,4,1,3,15,5,9,0,10,14,7,6,8,2,12},
                        {1,15,13,0,5,7,10,4,9,2,3,14,6,11,8,12}};

    //256 битовый ключ шифрования, разбитый на блоки по 32 бита
    std::uint32_t key[8];

    std::uint32_t concat(byte a, byte b, byte c, byte d);

    //Группировка 4 байт
    std::uint32_t concat(std::uint8_t arr[4]);

    //Входной файл
    std::string inputFile;

    //Выходной файловый поток
    std::ofstream output;

    //Мбютекс буффера
    std::mutex queueMutex;
    //Услованя переменная для блокировки потока шифрования, если буффер пуст
    std::condition_variable cv;

    //Очередь буфер для шифрования
    std::queue<std::pair<std::uint32_t,std::uint32_t>> *buffQueue; //buffer. First - lower, Second - Higher

    void reading(); //Поточная функция для чтения файла
    //Поточная функция для шифрования файла
    void crypt();
    //Функция для записи блока байт в файл
    //TODO: разбить данные на чанки!
    void write(std::pair<std::uint32_t,std::uint32_t> buff);
    //Флаг завершения чтения файла
    bool complete = false;

    template<typename INT>
    inline INT rol(INT var, size_t shift)
    {
        return (var << shift) | (var >> (sizeof(var)*CHAR_BIT-shift));
    }


    std::pair<std::uint32_t, std::uint32_t> oneStepCrypto(std::pair<std::uint32_t, std::uint32_t> buf, size_t &round);

    unsigned short mode;
};

#endif // GOST_H
