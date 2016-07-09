/*!
    MainWindow - class representing the main window.

    All functions that can be called up via the menu or
    toolbar, are described here. It includes rendering,
    printing, saving, calling other windows, error output, etc.

    The Font Editor window is represented by class FontEditor,
    the Settings windows - by class PreferencesDialog. Class
    SvgView is used to display a sheet with handwritten text
    and to place characters on a sheet correctly.
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QTime>
#include <QtCore/QTextStream>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QLabel>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QtGui/QWheelEvent>

#include "preferencesdialog.h"
#include "fontdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
      bool eventFilter(QObject *obj, QEvent *ev);

private:
    const QString version = "0.8.3 beta";

    enum ToolButton : int {
        Render,
        Print,
        Save,
        Separator,
        Previous,
        SheetNumber,
        Next
    };

    Ui::MainWindow *ui;
    PreferencesDialog *preferencesDialog;
    FontDialog *fontDialog;
    QErrorMessage *errorMessage;
    QLabel *sheetNumberLabel;

    QVector<int> sheetPointers; //pointers to the first character of the sheets
    int currentSheetNumber;     //number of sheet that is displaying or rendering now
    QString text;

    void saveAllSheetsToImages(const QString &fileName, const int indexOfExtension);
    void saveAllSheetsToPDF(const QString &fileName);
    void preparePrinter(QPrinter *printer);
    QString simplifyEnd(const QString &str); //returns string without whitespaces at the end

private slots:
    void showAboutBox();
    void showLicensesBox();
    void showHowToBox();
    void renderFirstSheet();
    void renderNextSheet();
    void renderPreviousSheet();
    void loadFont();
    void saveSheet(QString fileName = QString());
    void saveAllSheets();
    void printSheet();
    void printAllSheets(QPrinter *printer = new QPrinter(QPrinter::HighResolution));
    void loadTextFromFile();
    void loadSettings();
    void countMissedCharacters();
    void showSheetNumber(int number);
};

#endif // MAINWINDOW_H
