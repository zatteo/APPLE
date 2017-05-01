#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "serveur.h"
#include "automate.h"

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
    void on_sound_2_released();
    void FPlay();
    void FPause();

private:
    Ui::MainWindow *ui;
    Serveur s;
    QStateMachine * etat;

    QState * play;
    QState * pause;
    QState * avance_rapide;
    QState * retour_rapide;
    QState * start;
};

#endif // MAINWINDOW_H
