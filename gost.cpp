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

void gost::start(std::string inputFile, std::string outputFile, ushort mode)
{

//    ОСНОВНОЙ ЦИКЛ

//    buffQueue = new std::queue<std::pair<std::uint32_t,std::uint32_t>>;
//    this->inputFile = inputFile;
//    this->mode = mode;
//    output.open(outputFile, std::ios_base::binary);
//    std::thread([this](){reading();}).detach();
//    std::thread([this](){crypt();}).join();
//    delete buffQueue;

//ДЛЯ ТЕСТА НА ОДНОМ БЛОКЕ ДАННЫХ

    //69e6442d
    //2b2f6753

    this->mode = 0;
    //std::uint32_t a = 0xa709e291, b = 0x04cba6b5;
    //std::uint32_t a = 0xa92501d3, b = 0x66274d55;
    std::uint32_t a = 0x21043b04, b = 0x30043204;
    auto r = std::make_pair(a,b);

    for (size_t x = 0; x < 32;)
        r = oneStepCrypto(r, x);
    std::cout << std::hex << r.first << "\n" << r.second << std::endl;
}

void gost::crypt()
{
    size_t round = 0;

    std::pair<std::uint32_t, std::uint32_t> chank;

    while(true)
    {
        std::unique_lock<std::mutex> lk(queueMutex);
        cv.wait(lk,[this](){return !buffQueue->empty();});
        auto x = buffQueue->front();
        buffQueue->pop();


        chank = x;

        for (round = 0; round < 32;)
           chank = oneStepCrypto(chank, round);


        write(chank);

        if(buffQueue->empty() && complete)
            return;
    }
}

void gost::reading()
{
    std::ifstream file(this->inputFile, std::ios_base::binary);
    std::uint8_t buf[4];
    std::uint32_t lower,higher;
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

std::uint32_t gost::concat(std::uint8_t arr[4])
{
    std::uint32_t result = 0x00000000;
    for (size_t i = 0; i < 4; i++)
        result |= (static_cast<std::uint32_t>(arr[i]) << (8 * (3 - i)));
    return result;
}

void gost::write(std::pair<std::uint32_t, std::uint32_t> x)
{
    unsigned char *arr = reinterpret_cast<unsigned char*>(&x.first);
    for (size_t i = 0; i < 4; i++)
        output << arr[i];

    arr = reinterpret_cast<unsigned char*>(&x.second);

    for (size_t i = 0; i < 4; i++)
        output << arr[i];


}

std::pair<std::uint32_t, std::uint32_t> gost::oneStepCrypto(std::pair<std::uint32_t, std::uint32_t> buf, size_t &round)
{
    std::uint32_t lower, high; //Блок входных данных, разбитых на две половины

    unsigned char* byteBuffer;//Указатель для табля побайтовой работы с таблицой

    lower = buf.first;
    high = buf.second;

    if (this->mode == SIMPLECRYPT)
        lower = (lower +  (round < 24 ? key[round % 8] : key[31 - round])) % 4294967296;
    else if (this->mode == SIMPLEENCRYPT)
        lower = (lower +  (round < 8? key[round] : key[7 - (round - 8) % 8])) % 4294967296;



    byteBuffer = reinterpret_cast<unsigned char*>(&lower);

    //ЗАМЕНА ПО ТАБЛИЦЕ
    for (short i = 3, n = 7; i >= 0; i--, n -= 2)
        byteBuffer[i] = (((table[n][byteBuffer[i] >> 4]) << 4) | table[n - 1][(byteBuffer[i] & 0x0F)]);


    lower = rol(lower,11);
   // lower = (lower << 11) | (lower >> 21);
    lower = high ^ lower;
    round++;
    return (round != 31 ? std::make_pair(lower, buf.first): std::make_pair(buf.first, lower));
}

gost::~gost()
{
    if (output.is_open())
        output.close();
}
