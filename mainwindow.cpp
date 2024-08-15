#include <iostream>

#include "mainwindow.h"
#include "ui_MainWindow.h"

#include "ssh/shell.h"

MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_connectButton_clicked() {
    std::cout << "Connecting..." << std::endl;

    std::string ip = ui->ipEdit->text().toStdString();
    std::string username = ui->usernameEdit->text().toStdString();
    std::string password = ui->passwordEdit->text().toStdString();

    RemoteShell connection(ip, username, password);
    std::cout << "Connected!" << std::endl;

    std::cout << "output: " << connection.readOutput() << std::endl;

    ui->loginAndTerminal->setCurrentIndex(1);
}
