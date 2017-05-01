#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serveur.h"
#include "automate.h"
#include <QMainWindow>
#include <QListWidgetItem>
#include <QJsonObject>

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
    void on_sound_2_released();
    void FPlay();
    void FPause();
    void Beginning();
    void Update(QListWidgetItem *item);
    void UpdateInt(QJsonObject json);
    void add_liste_musique(QString nom);
    void add_liste_groupe(QString nom);

private:
    Ui::MainWindow *ui;
    Serveur * s;
    QStateMachine * etat;

    QState * play;
    QState * pause;
    QState * avance_rapide;
    QState * retour_rapide;
    QState * start;
    QState * next;
    QState * previous;

signals:
    void SPlay();
};

#endif // MAINWINDOW_H
