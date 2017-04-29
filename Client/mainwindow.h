#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "serveur.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_lecture_valueChanged(int value);

    void on_play_2_released();

    void on_sound_2_released();

private:
    Ui::MainWindow *ui;
    Serveur s;
};

#endif // MAINWINDOW_H
