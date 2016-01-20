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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void showAboutBox();
    void showLicensesBox();
    void render();
    void renderNextSheet();
    void renderPreviousSheet();
    void loadFont();
    void saveSheet(QString fileName = QString());
    void saveAllSheets();
    void printSheet();

private:
    Ui::MainWindow *ui;
    PreferencesDialog * preferencesDialog;
    QVector<int> sheetPointers;
    int currentSheetNumber;
    QString version;

signals:
};

#endif // MAINWINDOW_H
