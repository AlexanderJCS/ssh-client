#include "utils.h"

#include <QApplication>
#include <QFile>
#include <iostream>


void addStylesheet(QApplication& app, const std::string& filepath) {
    QFile file(filepath.c_str());

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        std::cout << "File opened!" << std::endl;

        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        app.setStyleSheet(styleSheet);
    } else {
        std::cout << "File not opened!" << std::endl;
    }
}
