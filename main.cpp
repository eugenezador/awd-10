#include "awd.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    awd w;
    w.show();
    return a.exec();
}
