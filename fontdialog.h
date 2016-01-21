#ifndef FONTDIALOG_H
#define FONTDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QSettings>
#include <QTextCodec>

namespace Ui {
class FontDialog;
}

class FontDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FontDialog(QWidget *parent = 0);
    ~FontDialog();

signals:
    void fontReady();
private:
    Ui::FontDialog *ui;
    QString fontFileName;
    QMultiMap<QChar, QString> font;
private slots:
    void loadFont();
    void loadletters();
    void saveFont();
    void decline();

    void limitTextEdit();

};

#endif // FONTDIALOG_H
