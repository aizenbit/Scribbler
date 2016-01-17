#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPrinter>

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
    void test_render();
    void loadFont();
    void saveSheet();
    void printSheet();

private:
    Ui::MainWindow *ui;
    PreferencesDialog * preferencesDialog;

signals:
};

#endif // MAINWINDOW_H
