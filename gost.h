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

class gost
{
public:
    gost();
    void set_key(std::string filepath);
    void start(std::string inputFile);

private:

    using byte = std::uint8_t;

    int table[8][16] = {{4,10,9,2,13,8,0,14,6,11,1,12,7,15,5,3}, //Таблица S
                        {14,11,4,12,6,13,15,10,2,3,8,1,0,7,5,9},
                        {5,8,1,13,10,3,4,2,14,15,12,7,6,0,9,11},
                        {7,13,10,1,0,8,9,15,14,4,6,12,11,2,5,3},
                        {6,12,7,1,5,15,13,8,4,10,9,14,0,3,11,2},
                        {4,11,10,0,7,2,1,13,3,6,8,5,9,12,15,14},
                        {13,11,4,1,3,15,5,9,0,10,14,7,6,8,2,12},
                        {1,15,13,0,5,7,10,4,9,2,3,14,6,11,8,12}};
    std::uint32_t key[8];
    std::uint32_t concat(byte a, byte b, byte c, byte d);
    std::uint32_t concat(std::uint8_t arr[4]);

    std::string inputFile;

    std::mutex queueMutex;
    std::condition_variable cv;

    std::queue<std::pair<std::uint32_t,std::uint32_t>> *buffQueue; //buffer. First - lower, Second - Higher

    template <typename T> //Циклический сдвиг влево
    inline T rotl(T x, size_t sk)
    {
        return (x << sk) | (x >> (sizeof(T) * 8 - sk));
    }

    template <typename T> //Циклический сдвиг вправо
    inline T rotr(T x, size_t sk)
    {
        return (x >> sk) | (x << (sizeof(T) * 8 - sk));
    }

    void reading();
    void crypt();

    bool complete = false;
};

#endif // GOST_H
