#ifndef FONTDIALOG_H
#define FONTDIALOG_H

#include <QtCore/QSettings>
#include <QtCore/QTextCodec>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QTreeWidgetItem>
#include <QtSvg/QSvgWidget>
#include <QtSvg/QSvgRenderer>

#include "letter.h"

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
    QSvgWidget *svgWidget;
    QSvgRenderer *svgRenderer;

    QString fontFileName;
    QMultiMap<QChar, Letter> font;

private slots:
    void loadFont();
    void loadLetters();
    void saveFont();
    void rejectChanges();
    void deleteLetter();

    void limitTextEdit();
    void setTextFromItem(QTreeWidgetItem *item);

};

#endif // FONTDIALOG_H
