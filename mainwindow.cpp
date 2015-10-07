#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QWheelEvent>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    manuscript = new Manuscript();

    connect(ui->actionAbout_Scribbler, SIGNAL(triggered()),
            this, SLOT(showAboutBox()));

    ui->graphicsView->setScene(manuscript);
    ui->graphicsView->setMaximumSize(manuscript->sheetSize);
    ui->graphicsView->scale(0.3, 0.3);
    zoom = new Graphics_view_zoom(ui->graphicsView);
    zoom->set_modifiers(Qt::ControlModifier);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete manuscript;
    delete zoom;
}

void MainWindow::showAboutBox()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("About"));
    aboutBox.setText(tr("It's Scribbler. I can't say anything esle."));
    aboutBox.exec();
}
