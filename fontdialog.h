/*!
    FontDialog - class representing the Font Editor window.
    It is designed to create and edit handwritten font.

    The handwritten font is a folder with SVG images and INI file
    that contains information about the associations of characters
    and images, as well as some additional data. This data includes
    the limits, inPoint and outPoint. Limits show part of the symbol,
    which part of the symbol must be within the line. It's a rectangle
    with a height equal to the height of the line and the width
    equal to the width of the character. InPoint and outPoint are
    the points of entry and exit of connecting lines for the letter.

    Font Editor allows the user to:
      * Create a new font and open an existing one for editing;
      * Upload an image to the font or remove it;
      * Automatically associate the symbol and image by analyzing
        the name of the image.
      * Edit additional data.

    Class SymbolDataEditor is used dor editing the additional data and
    displaying the selected character.
*/
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

#include "symboldata.h"

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
    QMultiMap<QChar, SymbolData> font;

private slots:
    void loadFont();
    void addNewSymbols();
    void saveFont();
    void rejectChanges();
    void deleteItem();
    void copyToChoosenSymbol();
    void autoLoadSymbols();

    void limitTextEdit();
    void setTextFromItem(QTreeWidgetItem *item);
    void loadFromEditorToFont();
    void enableDrawButtons(bool enable = true);
    void showTreeWidgetContextMenu(QPoint pos);
    QTreeWidgetItem * getSymbolItem(QChar key);
    QTreeWidgetItem * getCategoryItem(QChar key);
    bool isFileItem(QTreeWidgetItem *item);
    bool isSymbolItem(QTreeWidgetItem *item);
    bool isCategoryItem(QTreeWidgetItem *item);
};

#endif // FONTDIALOG_H
