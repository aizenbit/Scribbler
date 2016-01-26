#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QTime>
#include <QtCore/QTextStream>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>
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

private:
    Ui::MainWindow *ui;
    PreferencesDialog *preferencesDialog;
    FontDialog *fontDialog;
    QVector<int> sheetPointers; //pointers to the beginning of the sheets
    int currentSheetNumber;     //number of sheet that is displaying or rendering now
    QString version;            //program version
    void saveAllSheetsToImages(QString &fileName, int indexOfExtension);
    void saveAllSheetsToPDF(QString &fileName);
    void preparePrinter(QPrinter *printer);
    QString simplifyEnd(const QString &str);

private slots:
    void showAboutBox();
    void showLicensesBox();
    void renderFirstSheet();
    void renderNextSheet();
    void renderPreviousSheet();
    void loadFont();
    void saveSheet(QString fileName = QString());
    void saveAllSheets();
    void printSheet();
    void printAllSheets(QPrinter *printer = new QPrinter(QPrinter::HighResolution));
    void loadTextFromFile();
};

#endif // MAINWINDOW_H
