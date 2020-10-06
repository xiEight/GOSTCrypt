#include <QCoreApplication>
#include "gost.h"

int main(int argc, char *argv[])
{
    gost g;
    g.set_key("data");
    g.start("msg");
}
