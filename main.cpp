#include <QCoreApplication>
#include "gost.h"

int main(int argc, char *argv[])
{
//    if (argc == 4)
//    {
//        gost g;
//        g.set_key("data");
//        g.start(argv[1], argv[2], atoi(argv[3]));
//    }
//    else
//        std::cout << "Wrong Parametr List!" << std::endl;
    gost g;
   // g.set_key("data");
    g.start("MSG", "11MSG",1);
}
