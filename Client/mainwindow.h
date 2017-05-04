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
    void UpdateInt(QJsonObject json);

private slots:
    void on_sound_2_released();
    void FPlay();
    void FPause();
    void Update(QListWidgetItem *item);
    void add_liste_musique(QString nom);
    void add_liste_groupe(QString nom);
    QString intToTimer(int value);

    void on_volume_valueChanged(int value);

    void on_lecture_valueChanged(int position);

    void on_lecture_sliderPressed();

    void on_lecture_sliderReleased();

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
    QImage imageFromJson(const QJsonValue & val);

signals:
    void SPlay();
};

#endif // MAINWINDOW_H
