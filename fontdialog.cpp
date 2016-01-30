#include "fontdialog.h"
#include "ui_fontdialog.h"

FontDialog::FontDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FontDialog)
{
    ui->setupUi(this);

    connect(ui->choosenSymbolTextEdit, SIGNAL(textChanged()),
            this, SLOT(limitTextEdit()));
    connect(ui->fontFilePushButton, SIGNAL(clicked()),
            this, SLOT(loadFont()));
    connect(ui->SymbolFilesPushButton, SIGNAL(clicked()),
            this, SLOT(loadLetters()));
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(saveFont()));
    connect(ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(rejectChanges()));
    connect(ui->treeWidget, SIGNAL(itemPressed(QTreeWidgetItem*, int)),
            this, SLOT(setTextFromItem(QTreeWidgetItem*)));
    connect(ui->deleteSymbolButton, SIGNAL(pressed()),
            this, SLOT(deleteLetter()));
    connect(ui->drawInPointButton, SIGNAL(clicked()),
            ui->svgEditor, SLOT(enableInPointDrawing()));
    connect(ui->drawOutPointButton, SIGNAL(clicked()),
            ui->svgEditor, SLOT(enableOutPointDrawing()));
    connect(ui->drawLimitsButton, SIGNAL(clicked()),
            ui->svgEditor, SLOT(enableLimitsDrawing()));

    ui->SymbolFilesPushButton->setEnabled(false);
    ui->deleteSymbolButton->setEnabled(false);
    ui->drawInPointButton->setEnabled(false);
    ui->drawOutPointButton->setEnabled(false);
    ui->drawLimitsButton->setEnabled(false);
    ui->fontFileTextEdit->setLineWrapMode(QTextEdit::NoWrap);

    ui->treeWidget->setColumnCount(1);
    lastItem = nullptr;
}

FontDialog::~FontDialog()
{
    delete ui;
}

void FontDialog::loadFont()
{
    ui->svgEditor->setFixedHeight(ui->svgEditor->height());
    lastItem = nullptr;
    fontFileName.clear();
    fontFileName = QFileDialog::getSaveFileName(0, tr("Choose"), "",
                                                   tr("INI") +
                                                      "(*.ini);;" +
                                                   tr("All Files") +
                                                      "(*.*)");
    if (fontFileName.isEmpty())
        return;

    ui->fontFileTextEdit->setText(fontFileName);

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
        QTreeWidgetItem *letterItem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList(QString(key)));
        for (const Letter & value : font.values(key))
        {
            QTreeWidgetItem *valueItem = new QTreeWidgetItem(letterItem, QStringList(value.fileName));
            letterItem->addChild(valueItem);
        }
        ui->treeWidget->insertTopLevelItem(ui->treeWidget->topLevelItemCount(), letterItem);
    }
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

    QChar letter = ui->choosenSymbolTextEdit->toPlainText().at(0);

    QTreeWidgetItem * letterItem;

    if (font.contains(letter))
    {
        for (const Letter & value : font.values(letter))
            files += value.fileName;

        files.removeDuplicates();
        font.remove(letter);
        letterItem = ui->treeWidget->findItems(letter, Qt::MatchCaseSensitive).first();
        delete ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(letterItem));
    }

    letterItem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList(QString(letter)));

    letterItem->text(0);

    for (QString fileName : files)
    {
        Letter temp = { QFileInfo(fileName).fileName(),
                        QPointF(-1.0, -1.0),
                        QPointF(-1.0, -1.0),
                        QRectF(-1.0, -1.0, -1.0, -1.0) };
        font.insert(letter, temp);
    }

    for (QString value : files)
    {
        QTreeWidgetItem *valueItem = new QTreeWidgetItem(letterItem, QStringList(QFileInfo(value).fileName()));
        letterItem->addChild(valueItem);
    }

    ui->treeWidget->insertTopLevelItem(ui->treeWidget->topLevelItemCount(), letterItem);
    ui->choosenSymbolTextEdit->setText(QString());
}

void FontDialog::saveFont()
{
    QFile file (fontFileName);
    file.remove();

    QSettings fontSettings(fontFileName, QSettings::IniFormat);
    fontSettings.beginGroup("Font");
    fontSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    loadFromEditorToFont();
    lastItem = nullptr;

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
    ui->drawInPointButton->setEnabled(false);
    ui->drawOutPointButton->setEnabled(false);
    ui->drawLimitsButton->setEnabled(false);
    ui->SymbolFilesPushButton->setEnabled(false);
    ui->choosenSymbolTextEdit->clear();
    ui->fontFileTextEdit->clear();
    ui->treeWidget->clear();
    ui->svgEditor->disableDrawing(true);
    ui->svgEditor->drawLetter = false;
}

void FontDialog::limitTextEdit()
{
    QString text = ui->choosenSymbolTextEdit->toPlainText();

    if(!fontFileName.isEmpty())
    {
        ui->SymbolFilesPushButton->setEnabled(!text.isEmpty());
        ui->deleteSymbolButton->setEnabled(!text.isEmpty());
        ui->svgEditor->disableDrawing(true);
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

    if (item->parent() == nullptr)
    {
        ui->choosenSymbolTextEdit->setText(item->text(0));
        ui->svgEditor->drawInPoint = false;
        ui->svgEditor->drawOutPoint = false;
        ui->svgEditor->drawLimits = false;
        lastItem = nullptr;
    }
    else
    {
        ui->drawInPointButton->setEnabled(true);
        ui->drawOutPointButton->setEnabled(true);
        ui->drawLimitsButton->setEnabled(true);
        ui->choosenSymbolTextEdit->setText(item->parent()->text(0));
        ui->svgEditor->load(QFileInfo(fontFileName).path() + "//" + item->text(0));
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
    QChar letter = ui->choosenSymbolTextEdit->toPlainText().at(0);
    QList<QTreeWidgetItem *> letterItemList = ui->treeWidget->findItems(letter, Qt::MatchCaseSensitive);

    if (letterItemList.isEmpty())
        return;

    QTreeWidgetItem *letterItem = letterItemList.first();
    delete ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(letterItem));
    font.remove(letter);
}
