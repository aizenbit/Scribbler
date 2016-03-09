#include "fontdialog.h"
#include "ui_fontdialog.h"

FontDialog::FontDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FontDialog)
{
    ui->setupUi(this);
    buttonGroup = new QButtonGroup();
    buttonGroup->addButton(ui->drawInPointButton);
    buttonGroup->addButton(ui->drawOutPointButton);
    buttonGroup->addButton(ui->drawLimitsButton);

    contextMenu = new QMenu();
    contextMenu->addAction(tr("Delete"));
    contextMenu->addAction(tr("Copy to choosen symbol"));

    connect(ui->choosenSymbolTextEdit, SIGNAL(textChanged()),
            this, SLOT(limitTextEdit()));
    connect(ui->fontFilePushButton, SIGNAL(clicked()),
            this, SLOT(loadFont()));
    connect(ui->SymbolFilesPushButton, SIGNAL(clicked()),
            this, SLOT(loadLetters()));
    connect(ui->autoLoadPushButton, SIGNAL(clicked()),
            this, SLOT(autoLoadSymbols()));
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(saveFont()));
    connect(ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(rejectChanges()));
    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(setTextFromItem(QTreeWidgetItem*)));
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showTreeWidgetContextMenu(QPoint)));
    connect(contextMenu->actions()[ContextAction::Delete], SIGNAL(triggered(bool)),
            this, SLOT(deleteLetter()));
    connect(contextMenu->actions()[ContextAction::Copy], SIGNAL(triggered(bool)),
            this, SLOT(copyToChoosenSymbol()));
    connect(ui->drawInPointButton, SIGNAL(toggled(bool)),
            ui->svgEditor, SLOT(enableInPointDrawing(bool)));
    connect(ui->drawOutPointButton, SIGNAL(toggled(bool)),
            ui->svgEditor, SLOT(enableOutPointDrawing(bool)));
    connect(ui->drawLimitsButton, SIGNAL(toggled(bool)),
            ui->svgEditor, SLOT(enableLimitsDrawing(bool)));

    ui->fontFileTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    ui->drawInPointButton->setCheckable(true);
    ui->drawOutPointButton->setCheckable(true);
    ui->drawLimitsButton->setCheckable(true);
    contextMenu->actions()[ContextAction::Copy]->setEnabled(false);
    ui->drawInPointButton->setIcon(QIcon("://dark_cyan_dot.png"));
    ui->drawOutPointButton->setIcon(QIcon("://dark_magnetta_dot.png"));
    ui->drawLimitsButton->setIcon(QIcon("://border.png"));
    ui->drawInPointButton->setToolTip(tr("In Point"));
    ui->drawOutPointButton->setToolTip(tr("Out Point"));
    ui->drawLimitsButton->setToolTip(tr("Limits"));
    ui->treeWidget->setColumnCount(1);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeWidget->setSortingEnabled(true);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
    ui->splitter->setSizes(QList <int> () << 200 << 350);
    lastItem = nullptr;

    ui->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
}

FontDialog::~FontDialog()
{
    delete ui;
    delete buttonGroup;
    delete contextMenu;
}

void FontDialog::loadFont()
{
    QString newFileName = QFileDialog::getSaveFileName(0, tr("Choose"), "",
                                                          tr("INI") +
                                                          "(*.ini);;" +
                                                          tr("All Files") +
                                                          "(*.*)",
                                                       0, QFileDialog::DontConfirmOverwrite);
    if (newFileName.isEmpty())
        return;
    else
        fontFileName = newFileName;

    lastItem = nullptr;
    ui->fontFileTextEdit->setText(fontFileName);
    ui->autoLoadPushButton->setEnabled(true);

    QSettings fontSettings(fontFileName, QSettings::IniFormat);
    fontSettings.beginGroup("Font");
    fontSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    if (fontSettings.allKeys().size() == 0)
    {
        fontSettings.endGroup();
        return;
    }

    font.clear();
    for (const QString &key : fontSettings.childKeys())
        for (const Letter &value : fontSettings.value(key).value<QList<Letter>>())
            font.insert(key.at(0).toLower(), value);

    //It's a dirty hack, which helps to distinguish uppercase and lowercase
    //letters on freaking case-insensetive Windows
    fontSettings.beginGroup("UpperCase");
    for (const QString &key : fontSettings.childKeys())
        for (const Letter &value : fontSettings.value(key).value<QList<Letter>>())
            font.insert(key.at(0).toUpper(), value);
    fontSettings.endGroup();

    fontSettings.endGroup();

    ui->treeWidget->clear();

    for (QChar key : font.uniqueKeys())
    {
        QTreeWidgetItem *letterItem = getSymbolItem(key);
        for (const Letter & value : font.values(key))
        {
            QTreeWidgetItem *valueItem = new QTreeWidgetItem(letterItem, QStringList(value.fileName));
            letterItem->addChild(valueItem);
        }
    }

    ui->svgEditor->load(QString());
    enableDrawButtons(false);
}

void FontDialog::loadLetters()
{
    QStringList files = QFileDialog::getOpenFileNames(0, tr("Choose"), "",
                                                         tr("SVG") +
                                                            "(*.svg);;" +
                                                         tr("All Files") +
                                                            "(*.*)");
    if (files.isEmpty())
        return;

    QChar key = ui->choosenSymbolTextEdit->toPlainText().at(0);

    if (font.contains(key))
        for (int i = 0; i < files.count(); i++)
            for (int j = 0; j < font.values(key).count(); j++)
                if (QFileInfo(files.at(i)).fileName() == font.values(key).at(j).fileName)
                {
                    files.removeAt(i);
                    i--;
                    break;
                }

    QTreeWidgetItem *symbolItem = getSymbolItem(key);

    for (QString fileName : files)
    {
        Letter newLetterData = { QFileInfo(fileName).fileName(),
                        QPointF(-1.0, -1.0),
                        QPointF(-1.0, -1.0),
                        QRectF(-1.0, -1.0, -1.0, -1.0) };
        font.insert(key, newLetterData);
        QTreeWidgetItem *newLetterItem = new QTreeWidgetItem(symbolItem, QStringList(newLetterData.fileName));
        symbolItem->addChild(newLetterItem);
        ui->treeWidget->setCurrentItem(newLetterItem);
        setTextFromItem(newLetterItem);
    }
}

void FontDialog::saveFont()
{
    if (fontFileName.isEmpty())
        return;

    QFile file (fontFileName);
    file.remove();

    QSettings fontSettings(fontFileName, QSettings::IniFormat);
    fontSettings.beginGroup("Font");
    fontSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    loadFromEditorToFont();

    for (QChar &key : font.keys())
        if (key.isUpper())
        {
            fontSettings.beginGroup("UpperCase");
            fontSettings.remove(key);
            fontSettings.setValue(key, QVariant::fromValue(font.values(key)));
            fontSettings.endGroup();
        }
        else
        {
            fontSettings.remove(key);
            fontSettings.setValue(key, QVariant::fromValue(font.values(key)));
        }

    fontSettings.endGroup();

    emit fontReady();
}

void FontDialog::rejectChanges()
{
    font.clear();
    fontFileName.clear();
    lastItem = nullptr;
    enableDrawButtons(false);
    ui->SymbolFilesPushButton->setEnabled(false);
    ui->autoLoadPushButton->setEnabled(false);
    ui->choosenSymbolTextEdit->clear();
    ui->fontFileTextEdit->clear();
    ui->treeWidget->clear();
    ui->svgEditor->disableDrawing();
    ui->svgEditor->hideAll();
}

void FontDialog::limitTextEdit()
{
    QString text = ui->choosenSymbolTextEdit->toPlainText();

    if(!fontFileName.isEmpty())
    {
        ui->SymbolFilesPushButton->setEnabled(!text.isEmpty());
        contextMenu->actions()[ContextAction::Copy]->setEnabled(!text.isEmpty());
    }

    if (text.length() > 1)
    {
        text = text.left(1);
        ui->choosenSymbolTextEdit->setText(text);
    }
}

void FontDialog::setTextFromItem(QTreeWidgetItem *item)
{
    loadFromEditorToFont();

    if (isSymbolItem(item) || isCategoryItem(item))
    {
        if (isSymbolItem(item))
            ui->choosenSymbolTextEdit->setText(item->text(0));

        enableDrawButtons(false);
        ui->svgEditor->hideAll();
        ui->svgEditor->disableDrawing();
        lastItem = nullptr;
    }
    else
    {
        enableDrawButtons(true);
        ui->choosenSymbolTextEdit->setText(item->parent()->text(0));
        ui->svgEditor->load(QFileInfo(fontFileName).path() + '/' + item->text(0));
        QList<Letter> letterList = font.values(item->parent()->text(0).at(0));

        for (const Letter &letter : letterList)
            if (letter.fileName == item->text(0))
            {
                ui->svgEditor->setLetterData(letter.inPoint, letter.outPoint, letter.limits);
                break;
            }

        lastItem = item;
    }
}

void FontDialog::loadFromEditorToFont()
{
    if (lastItem != nullptr)
    {
        Letter newLetter;
        newLetter.fileName = lastItem->text(0);
        newLetter.inPoint = ui->svgEditor->getInPoint();
        newLetter.outPoint = ui->svgEditor->getOutPoint();
        newLetter.limits = ui->svgEditor->getLimits();
        QChar key = lastItem->parent()->text(0).at(0);
        QList<Letter> letterList = font.values(key);

        for (Letter &letter : letterList)
            if (letter.fileName == lastItem->text(0))
            {
                letter = newLetter;
                break;
            }

        font.remove(key);

        for (const Letter &letter : letterList)
            font.insert(key, letter);
    }
}

void FontDialog::deleteLetter()
{
    QTreeWidgetItem *selectedItem = ui->treeWidget->itemAt(ui->treeWidget->mapFromGlobal(contextMenu->pos()));

    QChar key = isSymbolItem(selectedItem) ? selectedItem->text(0).at(0) : selectedItem->parent()->text(0).at(0) ;
    QTreeWidgetItem *symbolItem = getSymbolItem(key);
    QTreeWidgetItem *categoryItem = symbolItem->parent();

    if (isFileItem(selectedItem))
    {
        QList<Letter> letterList = font.values(key);

        for (const Letter &letterData : letterList)
            if (letterData.fileName == selectedItem->text(0))
            {
                font.remove(key, letterData);
                delete selectedItem;
                break;
            }

        if(symbolItem->childCount() == 0)
            delete symbolItem;
    }
    else
    {
        for (QTreeWidgetItem *item : selectedItem->takeChildren())
            delete item;

        delete selectedItem;
        font.remove(key);
    }

    if (categoryItem->childCount() == 0)
        delete categoryItem;

    enableDrawButtons(false);
    ui->svgEditor->hideAll();
    ui->svgEditor->disableDrawing();
    lastItem = nullptr;
}

void FontDialog::enableDrawButtons(bool enable)
{
    if (!enable && buttonGroup->checkedButton() != nullptr)
    {
        buttonGroup->setExclusive(false);
        buttonGroup->checkedButton()->setChecked(false);
        buttonGroup->setExclusive(true);
    }

    ui->drawInPointButton->setEnabled(enable);
    ui->drawOutPointButton->setEnabled(enable);
    ui->drawLimitsButton->setEnabled(enable);
}

void FontDialog::showTreeWidgetContextMenu(QPoint pos)
{
    if (ui->treeWidget->itemAt(pos) == nullptr)
        return;

    if (ui->treeWidget->itemAt(pos)->parent() == nullptr)
    {
        contextMenu->actions()[ContextAction::Copy]->setEnabled(false);
        contextMenu->actions()[ContextAction::Delete]->setEnabled(false);
    }
    else
    {
        contextMenu->actions()[ContextAction::Copy]->setEnabled(!ui->choosenSymbolTextEdit->toPlainText().isEmpty());
        contextMenu->actions()[ContextAction::Delete]->setEnabled(true);
    }

    contextMenu->exec(QCursor::pos());
}

void FontDialog::copyToChoosenSymbol()
{
    if (ui->choosenSymbolTextEdit->toPlainText().isEmpty())
        return;

    QTreeWidgetItem *selectedItem = ui->treeWidget->selectedItems().at(0);

    QChar key = isSymbolItem(selectedItem) ? selectedItem->text(0).at(0) : selectedItem->parent()->text(0).at(0);
    QChar newKey = ui->choosenSymbolTextEdit->toPlainText().at(0);
    QList<QTreeWidgetItem *> letterItemList = ui->treeWidget->findItems(newKey, Qt::MatchExactly);

    QTreeWidgetItem *topLevelItem = getSymbolItem(newKey);

    if (!isSymbolItem(selectedItem))
    {
        for (Letter &letterData : font.values(key))
            if (letterData.fileName == selectedItem->text(0))
            {
                if (font.contains(newKey, letterData))
                    return;

                font.insert(newKey, letterData);
                QTreeWidgetItem *newLetterItem = new QTreeWidgetItem(topLevelItem, QStringList(letterData.fileName));
                topLevelItem->addChild(newLetterItem);
                break;
            }
    }
    else
    {
        for (Letter &letterData : font.values(key))
        {
            if (font.contains(newKey, letterData))
                continue;

            font.insert(newKey, letterData);
            QTreeWidgetItem *newLetterItem = new QTreeWidgetItem(topLevelItem, QStringList(letterData.fileName));
            topLevelItem->addChild(newLetterItem);
        }
    }
}

void FontDialog::autoLoadSymbols()
{
    QStringList files = QFileDialog::getOpenFileNames(0, tr("Choose"), "",
                                                         tr("SVG") +
                                                            "(*.svg);;" +
                                                         tr("All Files") +
                                                            "(*.*)");
    if (files.isEmpty())
        return;

    QMap <QString, QChar> markNames;
    markNames.insert("colon.svg", ':');
    markNames.insert("slash.svg", '/');
    markNames.insert("backslash.svg", '\\');
    markNames.insert("question.svg", '?');
    markNames.insert("vertical.svg", '|');
    markNames.insert("asterisk.svg", '*');
    markNames.insert("less.svg", '<');
    markNames.insert("greater.svg", '>');
    markNames.insert("caret.svg", '^');

    QRegularExpression upLetters("^UP_._?[0-9]*\\.svg");
    upLetters.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    QRegularExpression marks("^._?[0-9]*\\.svg");
    marks.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

    for (QString fileName : files)
    {
        fileName = QFileInfo(fileName).fileName();
        QChar symbol;

        if (marks.match(fileName).hasMatch())
            symbol = fileName.at(0).toLower();

        if (upLetters.match(fileName).hasMatch())
            if (fileName.at(3).isLetter())
                symbol = fileName.at(3).toUpper();

        if (markNames.contains(fileName.toLower()))
            symbol = markNames[fileName.toLower()];

        if (symbol.isNull())
            continue;

        if (font.contains(symbol))
        {
            bool fileExists = false;
            for (int j = 0; j < font.values(symbol).count(); j++)
                if (QFileInfo(fileName).fileName() == font.values(symbol).at(j).fileName)
                {
                    fileExists = true;
                    break;
                }

            if (fileExists)
                continue;
        }

        QTreeWidgetItem *topLevelItem = getSymbolItem(symbol);

        Letter newLetterData = { fileName,
                                 QPointF(-1.0, -1.0),
                                 QPointF(-1.0, -1.0),
                                 QRectF(-1.0, -1.0, -1.0, -1.0) };
        font.insert(symbol, newLetterData);
        QTreeWidgetItem *newLetterItem = new QTreeWidgetItem(topLevelItem, QStringList(newLetterData.fileName));
        topLevelItem->addChild(newLetterItem);
        ui->treeWidget->setCurrentItem(newLetterItem);
        setTextFromItem(newLetterItem);
    }
}

QTreeWidgetItem * FontDialog::getSymbolItem(QChar key)
{
    QTreeWidgetItem *topLevelItem = nullptr;

    QTreeWidgetItem *categoryItem = getCategoryItem(key);

    for (int i = 0; i < categoryItem->childCount(); i++)
        if (categoryItem->child(i)->text(0).at(0) == key)
        {
            topLevelItem = categoryItem->child(i);
            break;
        }

    if (topLevelItem == nullptr)
    {
        topLevelItem = new QTreeWidgetItem(categoryItem, QStringList(QString(key)));
        categoryItem->addChild(topLevelItem);
    }

    return topLevelItem;
}

QTreeWidgetItem * FontDialog::getCategoryItem(QChar key)
{
    QTreeWidgetItem *categoryItem;
    QString category;

    category = tr("Other symbols");

    if (key.isNumber())
        category = tr("Numbers");

    if (key.isLetter())
        category = tr("Non-latin letters");

    if ((key >= 'a' && key <= 'z') ||
        (key >= 'A' && key <= 'Z'))
        category = tr("Latin letters");

    if (key.isMark())
        category = tr("Other marks");

    if (key.isPunct())
        category = tr("Punctuation marks");

    QList<QTreeWidgetItem *> itemList = ui->treeWidget->findItems(category, Qt::MatchExactly);

    if (itemList.isEmpty())
    {
        categoryItem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList(category));
        ui->treeWidget->insertTopLevelItem(ui->treeWidget->topLevelItemCount(), categoryItem);
    }
    else
        categoryItem = itemList.first();

    return categoryItem;
}

bool FontDialog::isSymbolItem(QTreeWidgetItem *item)
{
    if (isCategoryItem(item))
        return false;

   return item->childCount() > 0;
}

bool FontDialog::isCategoryItem(QTreeWidgetItem *item)
{
    if (item != nullptr)
        return item->parent() == nullptr;
    else
        return false;
}

bool FontDialog::isFileItem(QTreeWidgetItem *item)
{
    if (item != nullptr)
        return !(isCategoryItem(item) || isSymbolItem(item));
    else
        return false;

}
