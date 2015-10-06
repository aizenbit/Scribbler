#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionAbout_Scribbler, SIGNAL(triggered()),
            this, SLOT(showAboutBox()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showAboutBox()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("About"));
    aboutBox.setText(tr("It's Scribbler. I can't say anything esle."));
    aboutBox.exec();
}
