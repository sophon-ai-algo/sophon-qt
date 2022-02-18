#include "mainwindow.h"

#include <QApplication>

void qin_fun()
{
    QTextStream qin(stdin);
    QString qin_str;
    while(1)
    {
        qin >> qin_str;
        qDebug() << qin_str;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    //QThread * qin_thread = QThread::create(qin_fun);
    //qin_thread->start();
    return a.exec();
}
