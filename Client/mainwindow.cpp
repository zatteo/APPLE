#include "mainwindow.h"
#include "ui_mainwindow.h"

int play=0; //pause = 0
int mute=1; //mute = 0

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    s.connectMPV("/tmp/mpv-socket");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lecture_valueChanged(int value)
{
    int min= value/60;
    int sec= value-60*min;
    if(sec < 10)
        ui->current->setText(QString::number(min) + ":0" + QString::number(sec));
    else
        ui->current->setText(QString::number(min) + ":" + QString::number(sec));
}

void MainWindow::on_play_2_released()
{
    if(play == 0){
        play= 1;
        QPixmap pixmap("../ressources/play.png");
        QIcon ButtonIcon(pixmap);
        ui->play_2->setIcon(ButtonIcon);
    }
    else {
        play= 0;
        QPixmap pixmap("../ressources/pause.png");
        QIcon ButtonIcon(pixmap);
        ui->play_2->setIcon(ButtonIcon);
    }
}

void MainWindow::on_sound_2_released()
{
    if(mute == 0){
        mute= 1;
        QPixmap pixmap("../ressources/sound.png");
        QIcon ButtonIcon(pixmap);
        ui->sound_2->setIcon(ButtonIcon);
    }
    else {
        mute= 0;
        QPixmap pixmap("../ressources/nosound.png");
        QIcon ButtonIcon(pixmap);
        ui->sound_2->setIcon(ButtonIcon);
    }
}
