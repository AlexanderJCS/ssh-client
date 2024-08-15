#ifndef QTTEST2_MAINWINDOW_H
#define QTTEST2_MAINWINDOW_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:
    void on_connectButton_clicked();

private:
    Ui::MainWindow *ui;
};


#endif //QTTEST2_MAINWINDOW_H
