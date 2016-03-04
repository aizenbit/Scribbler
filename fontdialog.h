#ifndef FONTDIALOG_H
#define FONTDIALOG_H

#include <QtCore/QSettings>
#include <QtCore/QTextCodec>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>

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
    enum ContextAction : int {
        Delete,
        Copy
    };

    Ui::FontDialog *ui;
    QButtonGroup *buttonGroup;
    QTreeWidgetItem *lastItem;
    QMenu *contextMenu;

    QString fontFileName;
    QMultiMap<QChar, Letter> font;

private slots:
    void loadFont();
    void loadLetters();
    void saveFont();
    void rejectChanges();
    void deleteLetter();
    void copyToChoosenSymbol();

    void limitTextEdit();
    void setTextFromItem(QTreeWidgetItem *item);
    void loadFromEditorToFont();
    void enableDrawButtons(bool enable = true);
    void showTreeWidgetContextMenu(QPoint pos);
    QTreeWidgetItem * getTopLevelItem(const QChar key);

};

#endif // FONTDIALOG_H
