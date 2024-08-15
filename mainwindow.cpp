#include <iostream>
#include <QThread>

#include "mainwindow.h"
#include "ui_MainWindow.h"

#include "ssh/shell.h"

MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    shell = nullptr;
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_connectButton_clicked() {
    std::string ip = ui->ipEdit->text().toStdString();
    std::string username = ui->usernameEdit->text().toStdString();
    std::string password = ui->passwordEdit->text().toStdString();

    QThread* thread = new QThread();
    shell = new RemoteShell(ip, username, password, nullptr);
    shell->moveToThread(thread);

    connect(thread, &QThread::started, shell, &RemoteShell::start);
    connect(shell, &RemoteShell::newOutput, this, &MainWindow::updateConsole);
    connect(thread, &QThread::finished, shell, &RemoteShell::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();

    ui->loginAndTerminal->setCurrentIndex(1);
}

void MainWindow::updateConsole(const QString& output) {
    ui->console->appendPlainText(output);
}
