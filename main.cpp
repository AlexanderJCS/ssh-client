#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <iostream>
#include <libssh/libssh.h>
#include "qtutils/utils.h"


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    addStylesheet(app, "../style.qss");

    MainWindow w;
    w.show();
    return QApplication::exec();
}
