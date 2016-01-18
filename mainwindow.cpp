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
    connect(ui->actionPrint_Sheet, SIGNAL(triggered()),
            this, SLOT(printSheet()));

    preferencesDialog->loadSettingsFromFile();
    preferencesDialog->loadSettingsToFile();
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
    ui->svgView->renderTextToImage(ui->textEdit->toPlainText()).save(fileName);
}

void MainWindow::printSheet()
{
    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    QSizeF paperSize(settings.value("sheet-width", 210.0).toInt(), settings.value("sheet-height", 297.0).toInt());
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer);
    if (dialog.exec() != QPrintDialog::Accepted)
        return;
    printer.setPaperSize(paperSize, QPrinter::Millimeter);
    printer.setResolution(settings.value("dpi", 300).toInt());
    bool isPortrait = settings.value("is-sheet-orientation-vertical", true).toBool();
    printer.setOrientation(isPortrait ? QPrinter::Portrait : QPrinter::Landscape);
    settings.endGroup();
    QPainter painter(&printer);
    painter.setRenderHint(QPainter::Antialiasing);
    QImage image = ui->svgView->renderTextToImage(ui->textEdit->toPlainText());
    if (image.format() == QImage::Format_Invalid || !printer.isValid())
        return;
    painter.drawImage(0,0,image);
    painter.end();

}
