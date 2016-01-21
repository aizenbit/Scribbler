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

    if (font.contains(letter))
    {
        files += font.values(letter);
        files.removeDuplicates();
        font.remove(letter);
    }

    for (QString fileName : files)
        font.insert(letter, QFileInfo(fileName).fileName());

    ui->choosenSymbolTextEdit->setText(QString());
}

void FontDialog::saveFont()
{
    QSettings fontSettings(fontFileName, QSettings::IniFormat);
    fontSettings.beginGroup("Font");
    fontSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    //QString fontDirectory = fontpath;
    //fontDirectory.remove(QRegularExpression("\\w+.\\w+$"));

    //font.clear();
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
