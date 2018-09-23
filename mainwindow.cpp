#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    preferencesDialog = new PreferencesDialog();
    fontDialog = new FontDialog();
    errorMessage = new QErrorMessage();
    sheetNumberLabel = new QLabel("<h2>1</h2>");
    sheetNumberLabel->setToolTip(tr("Number of Current Sheet"));
    sheetNumberLabel->setFrameShape(QFrame::Panel);
    sheetNumberLabel->setFrameShadow(QFrame::Sunken);
    sheetNumberLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    sheetNumberLabel->setMinimumWidth(ui->toolBar->height());
    errorMessage->setMinimumSize(200, 150);

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
            this, SLOT(loadSettings()));

    //----Help----
    connect(ui->actionAbout_Scribbler, SIGNAL(triggered()),
            this, SLOT(showAboutBox()));
    connect(ui->actionLicenses_and_Credits, SIGNAL(triggered()),
            this, SLOT(showLicensesBox()));
    connect(ui->actionHowTo, SIGNAL(triggered()),
            this, SLOT(showHowToBox()));

    //----ToolBar----
    //add actions to tool bar and connect them to slots
    connect(ui->toolBar->addAction(QPixmap("://render.png"), tr("Convert to Handwritten")), SIGNAL(triggered(bool)),
            this, SLOT(renderFirstSheet()));
    connect(ui->toolBar->addAction(QPixmap("://printer.png"), tr("Print Sheets")), SIGNAL(triggered(bool)),
            this, SLOT(printAllSheets()));
    connect(ui->toolBar->addAction(QPixmap("://save.png"), tr("Save Sheets")), SIGNAL(triggered(bool)),
            this, SLOT(saveAllSheets()));

    ui->toolBar->addSeparator();

    connect(ui->toolBar->addAction(QPixmap("://left.png"), tr("Previous Sheet")), SIGNAL(triggered(bool)),
            this, SLOT(renderPreviousSheet()));
    ui->toolBar->addWidget(sheetNumberLabel);
    connect(ui->toolBar->addAction(QPixmap("://right.png"), tr("Next Sheet")), SIGNAL(triggered(bool)),
            this, SLOT(renderNextSheet()));

    connect(fontDialog, SIGNAL(fontReady()),
            this, SLOT(updateCurrentSheet()));
    errorMessage->setModal(true);
    errorMessage->setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

    ui->toolBar->actions()[ToolButton::Render]->setShortcut(Qt::ControlModifier + Qt::Key_R);
    ui->toolBar->actions()[ToolButton::Print]->setShortcut(Qt::ControlModifier + Qt::Key_P);
    ui->toolBar->actions()[ToolButton::Save]->setShortcut(Qt::ControlModifier + Qt::Key_S);
    ui->toolBar->actions()[ToolButton::Next]->setShortcut(Qt::ControlModifier + Qt::Key_Right);
    ui->toolBar->actions()[ToolButton::Previous]->setShortcut(Qt::ControlModifier + Qt::Key_Left);
    ui->textEdit->installEventFilter(this);
    ui->toolBar->actions()[ToolButton::Next]->setDisabled(true);
    ui->toolBar->actions()[ToolButton::Previous]->setDisabled(true);

    //initialize some class members
    sheetPointers.push_back(0);
    currentSheetNumber = 0;

    preferencesDialog->loadSettingsFromFile();
    QTime dieTime = QTime::currentTime().addMSecs(1000);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    preferencesDialog->loadSettingsToFile();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete preferencesDialog;
    delete fontDialog;
    delete errorMessage;
    delete sheetNumberLabel;
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->textEdit && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->modifiers() == Qt::ControlModifier &&
                (keyEvent->key() == Qt::Key_Right ||
                 keyEvent->key() == Qt::Key_Left))
        {
            if (keyEvent->key() == Qt::Key_Right)
                 ui->toolBar->actions()[ToolButton::Next]->trigger();
            else
                 ui->toolBar->actions()[ToolButton::Previous]->trigger();

            return true;
        }
        else
            return false;
    }
    else
        // pass the event on to the parent class
        return QMainWindow::eventFilter(object, event);
}

void MainWindow::showAboutBox()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("About") + " Scribbler");
    aboutBox.setIconPixmap(QPixmap("://aboutIcon.png"));
    aboutBox.setText(tr("I'm one-eyed Blot and this is my favourite Scribbler in the universe. <br><br>"
                        "<strong>Scribbler</strong> ") + version + "<br>" +
                     tr("Qt version: ")+ QT_VERSION_STR);
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

                                   "Thanks to "
                                   "<a href=http://pro.guap.ru/privateoffice/main/462>Elizaveta Grebennikova</a>, "
                                   "<a href=https://github.com/aksenoff>aksenoff</a>, "
                                   "<a href=https://virink.com/domerk>Daniel Domerk</a>, "
                                   "<a href=https://github.com/dive155>dive155</a>, "
                                   "<a href=http://www.livelib.ru/reader/Azure_wave>Anastasiya Belozerskaya</a>, "
                                   "Julia and all "
                                   "<a href=https://github.com/aizenbit/Scribbler/graphs/contributors>contributors</a> for a help. =)"));
    aboutBox.exec();
}

void MainWindow::showHowToBox()
{
    QMessageBox howToBox;
    howToBox.setWindowTitle(tr("How To"));
    howToBox.setText(tr("<strong>Scribbler</strong> ") + version);
    howToBox.setInformativeText("<p>" + tr("Scribbler allows you to make handwritten text from a typed in text by simulating "
                                           "your own handwriting. It takes the symbols that you have prepared beforehand in "
                                           "a graphical editor, and places them on a sheet, which you can then save and print. "
                                           "If you’re too lazy to create own font, you can use the default one.<br><br>"
                                           "<strong>How to</strong>:"
                                           "<ol>"
                                           "<li>Draw the characters in any vector editor with the freehand tool and save them "
                                           "in SVG format.</li>"
                                           "<li>Use the font editor to open a font file in the same folder, where the "
                                           "SVG-files locates. If the font file doesn't exists, Scribbler will create it.</li>"
                                           "<li>Upload files via font editor, set the points of connection with nearby letters, "
                                           "line height and symbol width</li>"
                                           "<li>Convert text to handwritten using your font.</li>"
                                           "</ol>"
                                           "You can found the full manual here:<br>"
                                           "<a href=https://github.com/aizenbit/Scribbler/wiki/User-manual>"
                                           "https://github.com/aizenbit/Scribbler/wiki/User-manual</a>")+"</p>");
    howToBox.exec();
}

void MainWindow::on_actionShortcuts_triggered()
{
    QMessageBox shortcutsBox;
    shortcutsBox.setWindowTitle(tr("How To"));
    shortcutsBox.setText(tr("<strong>Scribbler</strong> ") + version);
    shortcutsBox.setInformativeText("<p>" + tr("<h3>Shortcuts:</h3>"
                                           "<strong>In the Main Window</strong>:"
                                           "<ol>"
                                           "<li><b>Ctrl + R</b> - Convert to handwritten</li>"
                                           "<li><b>Ctrl + P</b> - Print</li>"
                                           "<li><b>Ctrl + S</b> - Save</li>"
                                           "<li><b>Ctrl + →</b> - Next Sheet</li>"
                                           "<li><b>Ctrl + ←</b> - Previous sheet</li>"
                                           "</ol>"
                                           "<strong>In the Font Editor</strong>:"
                                           "<ol>"
                                           "<li><b>Alt + 1 </b> - Set the connection point with the previous letter</li>"
                                           "<li><b>Alt + 2</b> - Set the connection point with the next letter</li>"
                                           "<li><b>Alt + 3 </b> - Set the row height and the width of the letter</li>"
                                           "</ol>"
                                           "Also, you can found the full manual here:<br>"
                                           "<a href=https://github.com/aizenbit/Scribbler/wiki/User-manual>"
                                           "https://github.com/aizenbit/Scribbler/wiki/User-manual</a>")+"</p>");
    shortcutsBox.exec();
}

void MainWindow::renderFirstSheet()
{
    sheetPointers.clear();
    sheetPointers.push_back(0);

    currentSheetNumber = 0;

    text = ui->textEdit->toPlainText();
    text = simplifyEnd(text); //to avoid blank sheets at the end
    int endOfSheet = ui->svgView->renderText(QStringRef(&text));

    sheetPointers.push_back(endOfSheet);

    bool isThereMoreThanOneSheet = (text.length() - 1) >= endOfSheet;
    ui->toolBar->actions()[ToolButton::Next]->setEnabled(isThereMoreThanOneSheet);
    ui->toolBar->actions()[ToolButton::Previous]->setDisabled(true);

    countMissedCharacters();
    showSheetNumber(currentSheetNumber);
}

void MainWindow::renderNextSheet()
{
    if (!ui->toolBar->actions()[ToolButton::Next]->isEnabled())
        return;

    currentSheetNumber++;

    if (preferencesDialog->alternateMargins())
        ui->svgView->changeLeftRightMargins(currentSheetNumber % 2);
    else
        ui->svgView->changeLeftRightMargins(false);

    int lettersToTheEnd = text.length() - sheetPointers.at(currentSheetNumber);
    int endOfSheet = ui->svgView->renderText(QStringRef(&text, sheetPointers.at(currentSheetNumber), lettersToTheEnd));
    endOfSheet += sheetPointers.at(currentSheetNumber);

    ui->toolBar->actions()[ToolButton::Previous]->setEnabled(true);
    showSheetNumber(currentSheetNumber);

    if (endOfSheet >= text.length())    //i.e. this sheet is the last
    {
        ui->toolBar->actions()[ToolButton::Next]->setDisabled(true);
        return;
    }

    if (currentSheetNumber >= sheetPointers.count() - 1) //if this sheet has not yet been rendered,
        sheetPointers.push_back(endOfSheet);             //remember, where the next sheet begins
}

void MainWindow::renderPreviousSheet()
{
    if (!ui->toolBar->actions()[ToolButton::Previous]->isEnabled())
        return;

    currentSheetNumber--;

    if (preferencesDialog->alternateMargins())
        ui->svgView->changeLeftRightMargins(currentSheetNumber % 2);
    else
        ui->svgView->changeLeftRightMargins(false);

    int lettersToTheEnd = text.length() - sheetPointers.at(currentSheetNumber);
    ui->svgView->renderText(QStringRef(&text, sheetPointers.at(currentSheetNumber), lettersToTheEnd));

    ui->toolBar->actions()[ToolButton::Next]->setEnabled(true);

    if (currentSheetNumber == 0)
        ui->toolBar->actions()[ToolButton::Previous]->setDisabled(true);

    showSheetNumber(currentSheetNumber);
}

void MainWindow::updateCurrentSheet()
{
    ui->svgView->loadFont();
    int lettersToTheEnd = text.length() - sheetPointers.at(currentSheetNumber);
    ui->svgView->renderText(QStringRef(&text, sheetPointers.at(currentSheetNumber), lettersToTheEnd));
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
                                                       tr("PDF") +
                                                          "(*.pdf);;" +
                                                       tr("PNG") +
                                                          "(*.png)");

    QString extension = QRegularExpression("\\.\\w+$").match(fileName).captured();

    if (extension == ".png")
        saveAllSheetsToImages(fileName);

    if (extension == ".pdf")
        saveAllSheetsToPDF(fileName);
}

void MainWindow::saveAllSheetsToImages(const QString &fileName)
{
    int indexOfExtension = fileName.indexOf(QRegularExpression("\\.\\w+$"), 0);
    QString currentFileName;
    currentSheetNumber = -1;
    ui->toolBar->actions()[ToolButton::Next]->setEnabled(true);
    ui->svgView->hideBorders(true);

    while (ui->toolBar->actions()[ToolButton::Next]->isEnabled()) //while "Next Sheet" tool button is enabled,
    {                                                             //i.e. while rendering all sheets
        renderNextSheet();
        currentFileName = fileName;

        if (currentSheetNumber > 0 || ui->toolBar->actions()[ToolButton::Next]->isEnabled())
            //i.e. there is more than one sheet
            currentFileName.insert(indexOfExtension, QString("_%1").arg(currentSheetNumber));

        saveSheet(currentFileName);
    }

    ui->svgView->hideBorders(false);

    //we used renderNextSheet() for the first sheet instead of renderFirstSheet()
    //so we need to check the number of sheet and disable previous toolbutton if needed
    if (currentSheetNumber == 0)
        ui->toolBar->actions()[ToolButton::Previous]->setDisabled(true);
}

void MainWindow::saveAllSheetsToPDF(const QString &fileName)
{
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    preparePrinter(&printer);

    QPainter painter(&printer);
    painter.setRenderHint(QPainter::Antialiasing);

    currentSheetNumber = -1;
    ui->toolBar->actions()[ToolButton::Next]->setEnabled(true);
    ui->svgView->hideBorders(true);

    while (ui->toolBar->actions()[ToolButton::Next]->isEnabled()) //while "Next Sheet" tool button is enabled,
    {                                                             //i.e. while printing all sheets
        renderNextSheet();

        QImage image = ui->svgView->saveRenderToImage();

        if (image.format() == QImage::Format_Invalid || !printer.isValid())
            return;

        painter.drawImage(0, 0, image);

        if (ui->toolBar->actions()[ToolButton::Next]->isEnabled()) //if "Next Sheet" tool button is enabled,
            printer.newPage();                                     //i.e this sheet isn't the last one
    }

    painter.end();
    ui->svgView->hideBorders(false);

    //we used renderNextSheet() for the first sheet instead of renderFirstSheet()
    //so we need to check the number of sheet and disable previous toolbutton if needed
    if (currentSheetNumber == 0)
        ui->toolBar->actions()[ToolButton::Previous]->setDisabled(true);
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

void MainWindow::printAllSheets()
{
    QPrinter printer(QPrinter::HighResolution);

    QPrintDialog dialog(&printer);
    dialog.addEnabledOption(QAbstractPrintDialog::PrintPageRange);
    dialog.setMinMax(1, INT_MAX);

    if (dialog.exec() != QPrintDialog::Accepted)
        return;

    preparePrinter(&printer);

    QPainter painter(&printer);
    painter.setRenderHint(QPainter::Antialiasing);

    currentSheetNumber = -1;
    ui->toolBar->actions()[ToolButton::Next]->setEnabled(true);
    ui->svgView->hideBorders(true);

    while (ui->toolBar->actions()[ToolButton::Next]->isEnabled()) //while "Next Sheet" tool button is enabled,
    {                                                             //i.e. while printing all sheets
        renderNextSheet();

        if ((printer.fromPage() != 0 && printer.fromPage() > currentSheetNumber + 1)     //we're not going out of page range
				|| (printer.toPage() != 0 && printer.toPage() < currentSheetNumber + 1)) //defined by user
            continue;

        QImage image = ui->svgView->saveRenderToImage();

        if (image.format() == QImage::Format_Invalid || !printer.isValid())
            return;

        painter.drawImage(0, 0, image);

        if (ui->toolBar->actions()[ToolButton::Next]->isEnabled()) //if "Next Sheet" tool button is enabled,
            printer.newPage();                                     //i.e this sheet is not the last
	}

    painter.end();
    ui->svgView->hideBorders(false);

    //we used renderNextSheet() for the first sheet instead of renderFirstSheet()
    //so we need to check the number of sheet and disable previous toolbutton if needed
    if (currentSheetNumber == 0)
        ui->toolBar->actions()[ToolButton::Previous]->setDisabled(true);
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

void MainWindow::loadSettings()
{
    ui->svgView->loadSettingsFromFile();
    int sheetNumber = currentSheetNumber;
    renderFirstSheet();

    while (currentSheetNumber < sheetNumber && ui->toolBar->actions()[ToolButton::Next]->isEnabled())
        renderNextSheet();

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
	printer->setDoubleSidedPrinting(true);

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

void MainWindow::countMissedCharacters()
{
    QList<QChar> fontKeys = ui->svgView->getFontKeys();
    QSet <QChar> missedCharacters;

    for (const QChar &symbol : text)
        if (!fontKeys.contains(symbol) && !symbol.isSpace())
            missedCharacters << symbol;

    if (missedCharacters.isEmpty())
        return;

    QString errorText, missedCharactersString;
    int counter = 0;

    for (const QChar &symbol : missedCharacters)
    {
        missedCharactersString += QString("%1, ").arg(symbol);
        counter++;

        if (counter > 10)
            counter = 0;
    }
    missedCharactersString.remove(-2, 2);

    if (missedCharacters.count() == 1)
        errorText = tr("Character %1 is missing in the font! "
                       "Instead, there is space on the sheet.").arg(missedCharactersString);

    if (errorText.isEmpty() && missedCharacters.count() <= 10)
    {
        missedCharactersString.remove(-3, 1);
        missedCharactersString.insert(missedCharactersString.size() - 1, tr("and "));
        errorText = tr("Characters %1 are missing in the font! "
                       "Instead, there are spaces on the sheet.").arg(missedCharactersString);
    }

    if (errorText.isEmpty() && missedCharacters.count() > 10)
        errorText = tr("Some characters are missing in the font! "
                       "Instead, there are spaces on the sheet.<br><br>"
                       "List of missed characters:<br>") + missedCharactersString;

    errorMessage->showMessage(errorText, "Some symbols missed");
}

void MainWindow::showSheetNumber(int number)
{
    sheetNumberLabel->clear();
    sheetNumberLabel->setText(QString("<h2>%1</h2>").arg(number + 1));
}
