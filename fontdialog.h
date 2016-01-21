#ifndef FONTDIALOG_H
#define FONTDIALOG_H

#include <QDialog>

namespace Ui {
class FontDialog;
}

class FontDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FontDialog(QWidget *parent = 0);
    ~FontDialog();

private:
    Ui::FontDialog *ui;
};

#endif // FONTDIALOG_H
