#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // Create instance of QApplication and pass arguments from main().
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
