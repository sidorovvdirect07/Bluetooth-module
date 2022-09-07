#include "freeimucal.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FreeIMUCal w;
    w.show();
    return a.exec();
}
