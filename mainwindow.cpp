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
    preferencesDialog = new PreferencesDialog();

    connect(ui->actionAbout_Scribbler, SIGNAL(triggered()),
            this, SLOT(showAboutBox()));
    connect(ui->actionLoad_Font, SIGNAL(triggered()),
            this, SLOT(loadFont()));
    connect(ui->actionTest_rendering, SIGNAL(triggered()),
            this, SLOT(test_render()));
    connect(ui->actionPreferences, SIGNAL(triggered()),
            preferencesDialog, SLOT(exec()));
    connect(preferencesDialog, SIGNAL(settingsChanged()),
            ui->svgView, SLOT(loadSettingsFromFile()));
    connect(ui->actionSave_Sheet_as, SIGNAL(triggered()),
            this, SLOT(saveSheet()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete preferencesDialog;
}

void MainWindow::showAboutBox()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("About"));
    aboutBox.setIconPixmap(QPixmap("://aboutIcon.png"));
    aboutBox.setText(tr("I'm one-eyed Blot and this is my favourite Scribbler in the universe."));
    aboutBox.exec();
}

void MainWindow::test_render()
{
    ui->svgView->renderText(ui->textEdit->toPlainText());
}

void MainWindow::loadFont()
{
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open"), "",
                                              tr("INI") +
                                                 "(*.ini);;" +
                                              tr("All Files") +
                                                 "(*.*)");

    if (fileName.isEmpty())
        return;

    ui->svgView->loadFont(fileName);
}

void MainWindow::saveSheet()
{
    QString fileName = QFileDialog::getSaveFileName(0, tr("Save"), "",
                                              tr("PNG") +
                                                 "(*.png);;" +
                                              tr("All Files") +
                                                 "(*.*)");
    ui->svgView->renderTextToImage(ui->textEdit->toPlainText(), fileName);
}
