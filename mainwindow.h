#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "manuscript.h"
#include "graphics_view_zoom.h"

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

private:
    Ui::MainWindow *ui;
    Manuscript* manuscript;
    Graphics_view_zoom* zoom;

signals:
};

#endif // MAINWINDOW_H
