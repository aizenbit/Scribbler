#ifndef FONTDIALOG_H
#define FONTDIALOG_H

#include <QtCore/QSettings>
#include <QtCore/QTextCodec>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QTreeWidgetItem>

#include "letter.h"
#include "svgeditor.h"

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
    QButtonGroup *buttonGroup;
    QTreeWidgetItem *lastItem;

    QString fontFileName;
    QMultiMap<QChar, Letter> font;
    bool changes;

private slots:
    void loadFont();
    void loadLetters();
    void saveFont();
    void rejectChanges();
    void deleteLetter();

    void limitTextEdit();
    void setTextFromItem(QTreeWidgetItem *item);
    void loadFromEditorToFont();
    void enableDrawButtons(bool enable = true);

};

#endif // FONTDIALOG_H
