#include "gost.h"

gost::gost()
{

}

void gost::set_key(std::string filepath)
{
    std::ifstream file(filepath, std::ios_base::binary);
    std::uint8_t a,b,c,d;
    for (size_t i = 0; i < 8; i++)
    {
        file.read(reinterpret_cast<char*>(&a),1);
        file.read(reinterpret_cast<char*>(&b),1);
        file.read(reinterpret_cast<char*>(&c),1);
        file.read(reinterpret_cast<char*>(&d),1);
        key[i] = concat(a,b,c,d);

    }
}


std::uint32_t gost::concat(byte a, byte b, byte c, byte d)
{
    std::function<std::uint32_t(std::uint8_t)> func = [](std::uint8_t x) -> std::uint32_t
    {return static_cast<std::uint32_t>(x);};

    std::uint32_t result = func(a) << 24;
    result |= func(b) << 16;
    result |= func(c) << 8;
    result |= func(d);

    return result;
}

void gost::start(std::string inputFile)
{
    buffQueue = new std::queue<std::pair<std::uint32_t,std::uint32_t>>;
    this->inputFile = inputFile;
    std::thread([this](){reading();}).detach();
    std::thread([this](){crypt();}).join();
    delete buffQueue;
}

void gost::crypt()
{
    size_t round = 0;
    std::uint32_t lower, high; //Оптимизировать
    unsigned char* byteBuffer;
    std::vector<unsigned char> lowerBuff;
    while(true)
    {
        lowerBuff.clear();
        std::unique_lock<std::mutex> lk(queueMutex);
        cv.wait(lk,[this](){return !buffQueue->empty();});
        auto x = buffQueue->front();
        buffQueue->pop();
        lower = x.first;
        high = x.second;
        lower ^= key[round];
        byteBuffer = reinterpret_cast<unsigned char*>(&lower);

        for (size_t i = 0; i < 4; i++)
        {
            lowerBuff.push_back(table[2*i][byteBuffer[i] >> 4]);
            lowerBuff.push_back(table[2*i + 1][byteBuffer[i] & 0x0F]);
        }

        if(buffQueue->empty() && complete)
            return;
    }
}

void gost::reading()
{
    std::ifstream file(this->inputFile, std::ios_base::binary);
    std::uint32_t buf[4], lower,higher;
    while(!complete)
    {
        for (size_t i = 0; i < 4; i++)
            if (!file.eof())
                file.read(reinterpret_cast<char*>(&buf[i]),1);
            else
                buf[i] = 0x00;
        lower = concat(buf);
        for (size_t i = 0; i < 4; i++)
            if (!file.eof())
                file.read(reinterpret_cast<char*>(&buf[i]),1);
            else
            {
                buf[i] = 0x00;
                complete = true;
            }
        higher = concat(buf);
        queueMutex.lock();
        buffQueue->push(std::make_pair(lower,higher));
        queueMutex.unlock();
        cv.notify_one();
    }
}

std::uint32_t gost::concat(std::uint32_t arr[4])
{
    std::uint32_t result = 0x00000000;
    for (size_t i = 0; i < 4; i++)
        result |= (static_cast<std::uint32_t>(arr[i]) << (8 * (3 - i)));
    return result;
}
