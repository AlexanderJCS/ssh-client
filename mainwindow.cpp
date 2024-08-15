#include <iostream>
#include <QThread>

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

    QThread* thread = new QThread();
    RemoteShell* worker = new RemoteShell(ip, username, password, nullptr);
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &RemoteShell::start);
    connect(worker, &RemoteShell::newOutput, this, &MainWindow::updateConsole);
    connect(thread, &QThread::finished, worker, &RemoteShell::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();

    ui->loginAndTerminal->setCurrentIndex(1);
}

void MainWindow::updateConsole(const QString& output) {
    ui->console->appendPlainText(output);
}
