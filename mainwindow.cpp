#include <iostream>

#include "mainwindow.h"
#include "ui_MainWindow.h"

#include "ssh/connection.h"

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

    SSHConnection connection(ip, username, password);
    std::cout << "Connected!" << std::endl;

    ui->loginAndTerminal->setCurrentIndex(1);
}
