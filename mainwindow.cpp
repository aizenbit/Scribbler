#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    preferencesDialog = new PreferencesDialog();
    fontDialog = new FontDialog();

    //----File----
    connect(ui->actionConvert_to_Handwritten, SIGNAL(triggered()),
            this, SLOT(renderFirstSheet()));
    connect(ui->actionLoad_Text_from_File, SIGNAL(triggered()),
            this, SLOT(loadTextFromFile()));
    connect(ui->actionLoad_Font, SIGNAL(triggered()),
            this, SLOT(loadFont()));
    connect(ui->actionFont_Editor, SIGNAL(triggered()),
            fontDialog, SLOT(exec()));
    connect(ui->actionSave_Current_Sheet_as, SIGNAL(triggered()),
            this, SLOT(saveSheet()));
    connect(ui->actionSave_All_Sheets, SIGNAL(triggered()),
            this, SLOT(saveAllSheets()));
    connect(ui->actionPrint_Current_Sheet, SIGNAL(triggered()),
            this, SLOT(printSheet()));
    connect(ui->actionPrint_All, SIGNAL(triggered()),
            this, SLOT(printAllSheets()));

    //----Edit----
    //connect menu action "Show Toolbar"
    connect(ui->actionShow_ToolBar, SIGNAL(triggered(bool)),
            ui->toolBar, SLOT(setVisible(bool)));
    connect(ui->toolBar, SIGNAL(visibilityChanged(bool)),
            ui->actionShow_ToolBar, SLOT(setChecked(bool)));

    //preferencesDialog connections
    connect(ui->actionPreferences, SIGNAL(triggered()),
            preferencesDialog, SLOT(exec()));
    connect(preferencesDialog, SIGNAL(settingsChanged()),
            ui->svgView, SLOT(loadSettingsFromFile()));

    //----Help----
    connect(ui->actionAbout_Scribbler, SIGNAL(triggered()),
            this, SLOT(showAboutBox()));
    connect(ui->actionLicenses_and_Credits, SIGNAL(triggered()),
            this, SLOT(showLicensesBox()));

    //----ToolBar----
    //add actions to tool bar and connect them to slots
    connect(ui->toolBar->addAction(QPixmap("://render.png"), tr("Convert to Handwritten")), SIGNAL(triggered(bool)),
            this, SLOT(renderFirstSheet()));
    connect(ui->toolBar->addAction(QPixmap("://printer.png"), tr("Print Current Sheet")), SIGNAL(triggered(bool)),
            this, SLOT(printSheet()));
    connect(ui->toolBar->addAction(QPixmap("://save.png"), tr("Save Current Sheet as Image")), SIGNAL(triggered(bool)),
            this, SLOT(saveSheet()));

    ui->toolBar->addSeparator();

    connect(ui->toolBar->addAction(QPixmap("://right.png"), tr("Next Sheet")), SIGNAL(triggered(bool)),
            this, SLOT(renderNextSheet()));
    connect(ui->toolBar->addAction(QPixmap("://left.png"), tr("Previous Sheet")), SIGNAL(triggered(bool)),
            this, SLOT(renderPreviousSheet()));

    connect(fontDialog, SIGNAL(fontReady()),
            ui->svgView, SLOT(loadFont()));

    ui->toolBar->actions()[4]->setDisabled(true);
    ui->toolBar->actions()[5]->setDisabled(true);

    //initialize some class members
    sheetPointers.push_back(0);
    currentSheetNumber = 0;

    preferencesDialog->loadSettingsFromFile();

    /* This is a hack to avoid a bug. When program starts, it takes some time
     * (at least 1 ms on my configuration, but I set the delay to 100 ms just to make sure
     * that it will work on weaker machines) before it can write settings to file,
     * otherwise ui->colorButton->palette().background().color() will return
     * default buttons background color, which will be written to settings
     * file once program launches.
     */

    QTime dieTime = QTime::currentTime().addMSecs(100);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    preferencesDialog->loadSettingsToFile();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete preferencesDialog;
    delete fontDialog;
}

void MainWindow::showAboutBox()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("About") + " Scribbler");
    aboutBox.setIconPixmap(QPixmap("://aboutIcon.png"));
    aboutBox.setText(tr("I'm one-eyed Blot and this is my favourite Scribbler in the universe. <br><br>"
                        "<strong>Scribbler</strong> ") + version);
    aboutBox.setInformativeText("<p>" + tr("Distributed under The MIT License. See License and Credist page.") +
                                "<br><br>" + tr("Repository:") + "<br>"
                                "<a href=https://github.com/aizenbit/Scribbler>https://github.com/aizenbit/Scribbler<a></p>");
    aboutBox.exec();
}

void MainWindow::showLicensesBox()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("Licenses and Credits"));
    aboutBox.setText(tr("<strong>Scribbler</strong> ") + version);
    aboutBox.setInformativeText(tr("<p>The MIT License (MIT)<br><br>"
                                   "Copyright © 2016 <a href=https://github.com/aizenbit>aizenbit</a><br><br>"
                                   "Permission is hereby granted, free of charge, to any person obtaining a copy "
                                   "of this software and associated documentation files (the \"Software\"), to deal "
                                   "in the Software without restriction, including without limitation the rights "
                                   "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
                                   "copies of the Software, and to permit persons to whom the Software is "
                                   "furnished to do so, subject to the following conditions:<br><br>"

                                   "The above copyright notice and this permission notice shall be included in all "
                                   "copies or substantial portions of the Software.<br><br>"

                                   "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
                                   "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
                                   "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
                                   "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
                                   "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
                                   "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
                                   "SOFTWARE.<br><br>"

                                   "<strong>Credits:</strong><br>"
                                   "General Icon (\"one-eyed Blot\") made by <a href=https://virink.com/NH>Nuclear Hound</a> "
                                   "is licensed by <a href=https://www.gnu.org/licenses/gpl-3.0.html>GNU GPLv3</a>.<br>"
                                   "Tool Bar Icons made by <a href=http://www.flaticon.com/authors/picol title=Picol>Picol</a>, "
                                   "<a href=http://www.freepik.com title=Freepik>Freepik</a> from "
                                   "<a href=http://www.flaticon.com title=Flaticon>www.flaticon.com</a> "
                                   "are licensed by <a href=http://creativecommons.org/licenses/by/3.0/ "
                                   "title=Creative Commons BY 3.0>CC BY 3.0</a>.<br><br>"

                                   "Thanks to <a href=http://pro.guap.ru/privateoffice/main/462>Elizaveta Grebennikova</a>, "
                                   "<a href=https://github.com/aksenoff>aksenoff</a>, "
                                   "<a href=https://virink.com/domerk>Daniel Domerk</a>, "
                                   "<a href=http://www.livelib.ru/reader/Azure_wave>Anastasiya Belozerskaya</a> "
                                   "and grey eminence for a help. =)"));
    aboutBox.exec();
}

void MainWindow::renderFirstSheet()
{
    sheetPointers.clear();
    sheetPointers.push_back(0);

    currentSheetNumber = 0;
    ui->svgView->hideBorders(false);

    QString text = ui->textEdit->toPlainText();
    text = simplifyEnd(text);
    int endOfSheet = ui->svgView->renderText(QStringRef(&text));

    sheetPointers.push_back(endOfSheet);

    bool isThereMoreThanOneSheet = (text.length() - 1) >= endOfSheet;
    ui->toolBar->actions()[4]->setEnabled(isThereMoreThanOneSheet); //enable "Next Sheet" tool button
    ui->toolBar->actions()[5]->setDisabled(true);                   //disable "Previous Sheet" tool button
}

void MainWindow::renderNextSheet()
{
    currentSheetNumber++;
    ui->svgView->hideBorders(false);

    if (preferencesDialog->alternateMargins())
        ui->svgView->changeLeftRightMargins(currentSheetNumber % 2);
    else
        ui->svgView->changeLeftRightMargins(false);

    QString text = ui->textEdit->toPlainText();
    text = simplifyEnd(text);
    int lettersToTheEnd = text.length() - sheetPointers.at(currentSheetNumber);
    int endOfSheet = ui->svgView->renderText(QStringRef(&text, sheetPointers.at(currentSheetNumber), lettersToTheEnd));
    endOfSheet += sheetPointers.at(currentSheetNumber);

    ui->toolBar->actions()[5]->setEnabled(true); //enable "Previous Sheet" tool button

    if (endOfSheet >= text.length())    //this sheet is the last
    {
        ui->toolBar->actions()[4]->setDisabled(true); //disable "Next Sheet" tool button
        return;
    }

    if (currentSheetNumber >= sheetPointers.count() - 1) //if this sheet has not yet been rendered,
        sheetPointers.push_back(endOfSheet);             //remember, where the next sheet begins
}

void MainWindow::renderPreviousSheet()
{
    currentSheetNumber--;
    ui->svgView->hideBorders(false);

    if (preferencesDialog->alternateMargins())
        ui->svgView->changeLeftRightMargins(currentSheetNumber % 2);
    else
        ui->svgView->changeLeftRightMargins(false);

    QString text = ui->textEdit->toPlainText();
    int lettersToTheEnd = sheetPointers.at(currentSheetNumber) - sheetPointers.at(currentSheetNumber + 1);
    ui->svgView->renderText(QStringRef(&text, sheetPointers.at(currentSheetNumber), lettersToTheEnd));

    ui->toolBar->actions()[4]->setEnabled(true); //enable "Next Sheet" tool button

    if (currentSheetNumber == 0)
        ui->toolBar->actions()[5]->setDisabled(true); //disable "Previous Sheet" tool button
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

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    file.close();

    ui->svgView->loadFont(fileName);
}

void MainWindow::saveSheet(QString fileName)
{
    if (fileName.isEmpty())
        fileName = QFileDialog::getSaveFileName(0, tr("Save"), "",
                                                  tr("PNG") +
                                                     "(*.png);;" +
                                                  tr("All Files") +
                                                     "(*.*)");
    if (fileName.isEmpty())
        return;

    ui->svgView->hideBorders(true);
    ui->svgView->saveRenderToImage().save(fileName);
    ui->svgView->hideBorders(false);
}

void MainWindow::saveAllSheets()
{
    QString fileName = QFileDialog::getSaveFileName(0, tr("Save"), "",
                                                       tr("PNG") +
                                                          "(*.png);;" +
                                                       tr("PDF") +
                                                          "(*.pdf);;" +
                                                       tr("All Files") +
                                                          "(*.*)");
    int indexOfExtension = fileName.indexOf(QRegularExpression("\\.\\w+$"), 0);
    if (indexOfExtension == -1)
        return;

    if (fileName.mid(indexOfExtension) == ".png")
        saveAllSheetsToImages(fileName, indexOfExtension);

    if (fileName.mid(indexOfExtension) == ".pdf")
        saveAllSheetsToPDF(fileName);
}

void MainWindow::saveAllSheetsToImages(const QString &fileName, const int indexOfExtension)
{
    QString currentFileName;
    currentSheetNumber = -1;
    ui->toolBar->actions()[4]->setEnabled(true); //enable "Next Sheet" tool button

    while (ui->toolBar->actions()[4]->isEnabled()) //while "Next Sheet" tool button is enabled,
    {                                              //i.e. while rendering all sheets
        renderNextSheet();
        currentFileName = fileName;
        currentFileName.insert(indexOfExtension, QString("_%1").arg(currentSheetNumber));
        saveSheet(currentFileName);
    }
    ui->svgView->hideBorders(false);

    if (currentSheetNumber == 0)
        ui->toolBar->actions()[5]->setDisabled(true); //disable "Previous Sheet" tool button
}

void MainWindow::saveAllSheetsToPDF(const QString &fileName)
{
    QPrinter *printer = new QPrinter(QPrinter::PrinterResolution);
    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setOutputFileName(fileName);
    printAllSheets(printer);
}

void MainWindow::printSheet()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer);
    if (dialog.exec() != QPrintDialog::Accepted)
        return;

    preparePrinter(&printer);

    QPainter painter(&printer);
    painter.setRenderHint(QPainter::Antialiasing);

    ui->svgView->hideBorders(true);
    QImage image = ui->svgView->saveRenderToImage();
    ui->svgView->hideBorders(false);

    if (image.format() == QImage::Format_Invalid || !printer.isValid())
        return;

    painter.drawImage(0, 0, image);
    painter.end();

}

void MainWindow::printAllSheets(QPrinter *printer)
{
    if (printer->outputFormat() != QPrinter::PdfFormat)
    {
        QPrintDialog dialog(printer);
        if (dialog.exec() != QPrintDialog::Accepted)
            return;
    }

    preparePrinter(printer);

    QPainter painter(printer);
    painter.setRenderHint(QPainter::Antialiasing);

    currentSheetNumber = -1;
    ui->toolBar->actions()[4]->setEnabled(true); //enable "Next Sheet" tool button

    while (ui->toolBar->actions()[4]->isEnabled())//while "Next Sheet" tool button is enabled,
    {                                             //i.e. while printing all sheets
        renderNextSheet();
        ui->svgView->hideBorders(true);
        QImage image = ui->svgView->saveRenderToImage();
        if (image.format() == QImage::Format_Invalid || !printer->isValid())
        {
            delete printer;
            return;
        }
        painter.drawImage(0, 0, image);

        if (ui->toolBar->actions()[4]->isEnabled()) //if "Next Sheet" tool button is disabled,
            printer->newPage();                      //i.e this sheet is the last
    }

    painter.end();
    ui->svgView->hideBorders(false);

    delete printer;

    if (currentSheetNumber == 0)
        ui->toolBar->actions()[5]->setDisabled(true); //disable "Previous Sheet" tool button
}

void MainWindow::loadTextFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open"), "",
                                              tr("txt") +
                                                 "(*.txt);;" +
                                              tr("All Files") +
                                                 "(*.*)");

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName("UTF-8")); //TODO: add ability to change codec

    ui->textEdit->setText(in.readAll());
}

void MainWindow::preparePrinter(QPrinter *printer)
{
    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");

    QSizeF paperSize(settings.value("sheet-width", 210.0).toInt(), settings.value("sheet-height", 297.0).toInt());
    bool isPortrait = settings.value("is-sheet-orientation-vertical", true).toBool();

    printer->setPaperSize(paperSize, QPrinter::Millimeter);
    printer->setResolution(settings.value("dpi", 300).toInt());
    printer->setOrientation(isPortrait ? QPrinter::Portrait : QPrinter::Landscape);

    settings.endGroup();
}

QString MainWindow::simplifyEnd(const QString& str)
{
    int n = str.size() - 1;

    for (; n >= 0; --n)
        if (!str.at(n).isSpace())
            return str.left(n + 1);

    return "";
}
