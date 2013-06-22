#include "qappmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QAppMainWindow w;
    w.show();
    
    return a.exec();
}
