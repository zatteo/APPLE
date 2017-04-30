#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serveur.h"
#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class Serveur;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setMainWindow(MainWindow *window); // récupére la MainWindow

private slots:

    void on_lecture_valueChanged(int value);

    void on_play_2_released();

    void on_sound_2_released();

private:
    Ui::MainWindow *ui;
    Serveur *s;
};

#endif // MAINWINDOW_H
