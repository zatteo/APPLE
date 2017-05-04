#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serveur.h"
#include "automate.h"
#include <QMainWindow>
#include <QListWidgetItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QTranslator>

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
    QJsonArray getSongs();
    QJsonArray getPlaylists();
    QJsonArray getRadios();

    bool isTaglibPresent(QString title);
    bool isCoverPresent(QString title);

    void UpdateLocal(QString title);

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
    void AvanceRapide();
    void RetourRapide();
    void AvanceNormal();
    void NextSong();
    void getInfo();

    void on_liste_groupe_itemClicked(QListWidgetItem *item);

    void on_actionFrances_triggered();

    void on_actionEnglish_triggered();

private:
    Ui::MainWindow *ui;
    Serveur * s;
    QStateMachine * etat;

    QMap<QString, QJsonArray> asso_titre_musique;

    QState * play;
    QState * pause;
    QState * avance_rapide;
    QState * retour_rapide;
    QState * start;
    QState * next;
    QState * previous;
    QTimer * timer;
    QImage imageFromJson(const QJsonValue & val);

    QJsonArray songs; // liste des morceaux
    QJsonArray playlists; // liste des playlists
    QJsonArray radios; // liste des radios

    void songsAddTaglib(QString title, QJsonObject taglib);
    void songsAddCover(QString title, QString saveName);

    // i18n

    QString language; // "FR" || "EN"
    QJsonObject translations;

    void updateLanguage(QString language);

signals:
    void SPlay();
    void SPause();
};

#endif // MAINWINDOW_H
