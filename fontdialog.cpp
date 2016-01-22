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
            this, SLOT(loadletters()));
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(saveFont()));
    connect(ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(decline()));

    ui->SymbolFilesPushButton->setEnabled(false);

    ui->treeWidget->setColumnCount(1);
    /*QList<QTreeWidgetItem *> items;
    for (int i = 0; i < 10; ++i)
        items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("item: %1").arg(i))));
    for (QTreeWidgetItem * parent : items)
        parent->addChild(new QTreeWidgetItem(parent, QStringList(QString("item: %1").arg((int)parent))));
    ui->treeWidget->insertTopLevelItems(0, items);*/

}

FontDialog::~FontDialog()
{
    delete ui;
}

void FontDialog::loadFont()
{
    fontFileName.clear();
    fontFileName = QFileDialog::getSaveFileName(0, tr("Choose"), "",
                                                   tr("INI") +
                                                      "(*.ini);;" +
                                                   tr("All Files") +
                                                      "(*.*)");
    if (fontFileName.isEmpty())
        return;

    ui->SymbolFilesPushButton->setEnabled(true);

    QSettings fontSettings(fontFileName, QSettings::IniFormat);
    fontSettings.beginGroup("Font");
    fontSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    if (fontSettings.allKeys().size() == 0)
    {
        fontSettings.endGroup();
        return;
    }

    font.clear();
    for (QString & key : fontSettings.childKeys())
        for (QString & value : fontSettings.value(key).toStringList())
            font.insert(key[0].toLower(), value);

    //It's a dirty hack, which helps to distinguish uppercase and lowercase
    //letters on freaking case-insensetive Windows
    fontSettings.beginGroup("UpperCase");
    for (QString & key : fontSettings.childKeys())
        for (QString & value : fontSettings.value(key).toStringList())
            font.insert(key[0].toUpper(), value);
    fontSettings.endGroup();

    fontSettings.endGroup();

    ui->treeWidget->clear();

    for(QChar letter : font.uniqueKeys())
    {
        QTreeWidgetItem * letterItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(letter)));
        for (QString value : font.values(letter))
        {
            QTreeWidgetItem * valueItem = new QTreeWidgetItem(letterItem, QStringList(value));
            letterItem->addChild(valueItem);
        }
        ui->treeWidget->insertTopLevelItem(ui->treeWidget->topLevelItemCount(), letterItem);
    }
}

void FontDialog::loadletters()
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
        files += font.values(letter);
        files.removeDuplicates();
        font.remove(letter);
        letterItem = ui->treeWidget->findItems(letter, Qt::MatchCaseSensitive).first();
        delete ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(letterItem));
    }

    letterItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(letter)));

    for (QString fileName : files)
        font.insert(letter, QFileInfo(fileName).fileName());

    for (QString value : files)
    {
        QTreeWidgetItem * valueItem = new QTreeWidgetItem(letterItem, QStringList(QFileInfo(value).fileName()));
        letterItem->addChild(valueItem);
    }
    ui->treeWidget->insertTopLevelItem(ui->treeWidget->topLevelItemCount(), letterItem);

    ui->choosenSymbolTextEdit->setText(QString());
}

void FontDialog::saveFont()
{
    QSettings fontSettings(fontFileName, QSettings::IniFormat);
    fontSettings.beginGroup("Font");
    fontSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    for (QChar & key : font.keys())
        if (key.isUpper())
        {
            fontSettings.beginGroup("UpperCase");
            fontSettings.remove(key);
            fontSettings.setValue(key, QVariant(font.values(key)));
            fontSettings.endGroup();
        }
        else
        {
            fontSettings.remove(key);
            fontSettings.setValue(key, QVariant(font.values(key)));
        }

    fontSettings.endGroup();

    emit fontReady();
}

void FontDialog::decline()
{
    font.clear();
    fontFileName.clear();
    ui->SymbolFilesPushButton->setEnabled(false);
}

void FontDialog::limitTextEdit()
{
    QString text = ui->choosenSymbolTextEdit->toPlainText();
    if (text.length() > 1)
    {
        text = text.left(1);
        ui->choosenSymbolTextEdit->setText(text);
    }
}
