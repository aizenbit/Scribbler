#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QMessageBox>
#include <QWheelEvent>
#include <QScrollBar>
#include <QTime>

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

private slots:
    void showAboutBox();
    void showLicensesBox();
    void render();
    void renderNextSheet();
    void renderPreviousSheet();
    void loadFont();
    void saveSheet(QString fileName = QString());
    void saveAllSheets();
    void printSheet();
    void printAllSheets();

private:
    Ui::MainWindow *ui;
    PreferencesDialog * preferencesDialog;
    FontDialog * fontDialog;
    QVector<int> sheetPointers; //pointers to the beginning of the sheets
    int currentSheetNumber;     //number of sheet that is displaying or rendering now
    QString version;            //program version

signals:
};

#endif // MAINWINDOW_H
