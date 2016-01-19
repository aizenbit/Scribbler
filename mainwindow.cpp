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
    connect(ui->actionRender, SIGNAL(triggered()),
            this, SLOT(render()));
    connect(ui->actionPreferences, SIGNAL(triggered()),
            preferencesDialog, SLOT(exec()));
    connect(preferencesDialog, SIGNAL(settingsChanged()),
            ui->svgView, SLOT(loadSettingsFromFile()));
    connect(ui->actionSave_Sheet_as, SIGNAL(triggered()),
            this, SLOT(saveSheet()));
    connect(ui->actionPrint_Sheet, SIGNAL(triggered()),
            this, SLOT(printSheet()));

    connect(ui->actionShow_ToolBar, SIGNAL(triggered(bool)),
            ui->toolBar, SLOT(setVisible(bool)));
    connect(ui->toolBar, SIGNAL(visibilityChanged(bool)),
            ui->actionShow_ToolBar, SLOT(setChecked(bool)));

    connect(ui->toolBar->addAction("Render"), SIGNAL(triggered(bool)),
            this, SLOT(render()));
    connect(ui->toolBar->addAction("Save as Image"), SIGNAL(triggered(bool)),
            this, SLOT(saveSheet()));
    ui->toolBar->addSeparator();
    connect(ui->toolBar->addAction("Next"), SIGNAL(triggered(bool)),
            this, SLOT(renderNextSheet()));
    connect(ui->toolBar->addAction("Previous"), SIGNAL(triggered(bool)),
            this, SLOT(renderPreviousSheet()));

    ui->toolBar->actions()[4]->setDisabled(true);
    sheetPointers.push_back(0);
    currentSheetNumber = 0;

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
    aboutBox.setWindowTitle(tr("About") + " Scribbler");
    aboutBox.setIconPixmap(QPixmap("://aboutIcon.png"));
    aboutBox.setText(tr("I'm one-eyed Blot and this is my favourite Scribbler in the universe. <br><br>"
                        "<strong>Scribbler</strong> 0.1 alpha"));
    aboutBox.setInformativeText("<p>" + tr("Distributed under <a href=https://github.com/aizenbit/Scribbler/blob/master/LICENSE>The MIT License</a>.") +
                                "<br><br><a href=https://github.com/aizenbit/Scribbler>https://github.com/aizenbit/Scribbler<a></p>");
    aboutBox.exec();
}

void MainWindow::render()
{
    sheetPointers.clear();
    currentSheetNumber = 0;
    sheetPointers.push_back(0);
    QString text = ui->textEdit->toPlainText();
    int endOfSheet = ui->svgView->renderText(QStringRef(&text));
    sheetPointers.push_back(endOfSheet);
    /*for (int i = text.length(); i >= endOfSheet; i--)
    {
        if (!text.at(i).isSpace())
            break;

        text.remove(text.length() - 1, 1);
    }
    if ()*/
    ui->toolBar->actions()[3]->setEnabled(true);
    ui->toolBar->actions()[4]->setDisabled(true);
}

void MainWindow::renderNextSheet()
{
    QString text = ui->textEdit->toPlainText();
    currentSheetNumber++;
    int lettersToTheEnd = text.length() - sheetPointers.at(currentSheetNumber);
    int endOfSheet = ui->svgView->renderText(QStringRef(&text, sheetPointers.at(currentSheetNumber), lettersToTheEnd));
    endOfSheet += sheetPointers.at(currentSheetNumber);

    ui->toolBar->actions()[4]->setEnabled(true);

    if (endOfSheet >= text.length())
    {
        ui->toolBar->actions()[3]->setDisabled(true);
        return;
    }

    if (currentSheetNumber >= sheetPointers.count() - 1)
    {
        sheetPointers.push_back(endOfSheet);
    }
}

void MainWindow::renderPreviousSheet()
{
    QString text = ui->textEdit->toPlainText();
    currentSheetNumber--;
    int lettersToTheEnd = sheetPointers.at(currentSheetNumber) - sheetPointers.at(currentSheetNumber + 1);
    ui->svgView->renderText(QStringRef(&text, sheetPointers.at(currentSheetNumber), lettersToTheEnd));

    ui->toolBar->actions()[3]->setEnabled(true);

    if (currentSheetNumber == 0)
    {
        ui->toolBar->actions()[4]->setDisabled(true);
        return;
    }
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
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer);
    if (dialog.exec() != QPrintDialog::Accepted)
        return;

    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");

    QSizeF paperSize(settings.value("sheet-width", 210.0).toInt(), settings.value("sheet-height", 297.0).toInt());
    bool isPortrait = settings.value("is-sheet-orientation-vertical", true).toBool();

    printer.setPaperSize(paperSize, QPrinter::Millimeter);
    printer.setResolution(settings.value("dpi", 300).toInt());
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
