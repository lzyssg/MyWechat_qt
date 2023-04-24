#include "mainwindow.h"

#include <QApplication>
#include "logindialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    LoginDialog logindlg;
    logindlg.show();
    return a.exec();

}
