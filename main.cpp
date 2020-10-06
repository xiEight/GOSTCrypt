#include <QCoreApplication>
#include "gost.h"

int main(int argc, char *argv[])
{
    if (argc == 3)
    {
        gost g;
        g.set_key("data");
        g.start(argv[1], argv[2]);
    }
    else
        std::cout << "Wrong Parametr List!" << std::endl;
}
