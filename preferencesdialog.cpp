#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(loadSettingsToFile()));
    connect(ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(loadSettingsFromFile()));
    connect(ui->VRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(changeSheetOrientation()));
    connect(ui->colorButton, SIGNAL(clicked()),
            this, SLOT(setColor()));

    sheetSizeSignalMapper = new QSignalMapper(this);

    connect(ui->A4RadioButton, SIGNAL(clicked()),
            sheetSizeSignalMapper, SLOT(map()));
    connect(ui->A5RadioButton, SIGNAL(clicked()),
            sheetSizeSignalMapper, SLOT(map()));
    connect(ui->sheetHeightSpinBox, SIGNAL(valueChanged(int)),
            sheetSizeSignalMapper, SLOT(map()));
    connect(ui->sheetWidthSpinBox, SIGNAL(valueChanged(int)),
            sheetSizeSignalMapper, SLOT(map()));

    sheetSizeSignalMapper->setMapping(ui->A4RadioButton, (int)SheetSize::A4);
    sheetSizeSignalMapper->setMapping(ui->A5RadioButton, (int)SheetSize::A5);
    sheetSizeSignalMapper->setMapping(ui->sheetHeightSpinBox, (int)SheetSize::Custom);
    sheetSizeSignalMapper->setMapping(ui->sheetWidthSpinBox, (int)SheetSize::Custom);

    connect (sheetSizeSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(setSheetSize(int)));

    changedByProgram = false;
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
    delete sheetSizeSignalMapper;
}

void PreferencesDialog::loadSettingsToFile()
{
    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    settings.setValue("dpi", QVariant(ui->dpiSpinBox->value()));
    settings.setValue("letter-spacing", QVariant(ui->letterSpacingSpinBox->value()));
    settings.setValue("line-spacing", QVariant(ui->lineSpacingSpinBox->value()));
    settings.setValue("spaces-in-tab", QVariant(ui->spacesInTabSpinBox->value()));
    settings.setValue("font-size", QVariant(ui->fontSizeSpinBox->value()));
    settings.setValue("font-color", QVariant(ui->colorButton->palette().background().color().name()));
    settings.setValue("use-custom-font-color", QVariant(ui->fontColorCheckBox->isChecked()));
    settings.setValue("sheet-width", QVariant(ui->sheetWidthSpinBox->value()));
    settings.setValue("sheet-height", QVariant(ui->sheetHeightSpinBox->value()));
    settings.setValue("right-margin", QVariant(ui->rightMarginsSpinBox->value()));
    settings.setValue("left-margin", QVariant(ui->leftMarginsSpinBox->value()));
    settings.setValue("top-margin", QVariant(ui->topMarginsSpinBox->value()));
    settings.setValue("bottom-margin", QVariant(ui->bottomMarginsSpinBox->value()));
    settings.setValue("is-sheet-orientation-vertical", QVariant(ui->VRadioButton->isChecked()));
    settings.endGroup();

    emit settingsChanged();
}

void PreferencesDialog::loadSettingsFromFile()
{
    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    ui->dpiSpinBox->setValue(           settings.value("dpi", 300).toInt());
    ui->letterSpacingSpinBox->setValue( settings.value("letter-spacing", -1.5).toDouble());
    ui->lineSpacingSpinBox->setValue(   settings.value("line-spacing", 0.0).toDouble());
    ui->spacesInTabSpinBox->setValue(   settings.value("spaces-in-tab", 4).toInt());
    ui->fontSizeSpinBox->setValue(      settings.value("font-size", 6.0).toDouble());
    ui->sheetWidthSpinBox->setValue(    settings.value("sheet-width", 148).toInt());
    ui->sheetHeightSpinBox->setValue(   settings.value("sheet-height", 210).toInt());
    ui->rightMarginsSpinBox->setValue(  settings.value("right-margin", 20).toInt());
    ui->leftMarginsSpinBox->setValue(   settings.value("left-margin", 10).toInt());
    ui->topMarginsSpinBox->setValue(    settings.value("top-margin", 10).toInt());
    ui->bottomMarginsSpinBox->setValue( settings.value("bottom-margin", 5).toInt());
    ui->VRadioButton->setChecked(       settings.value("is-sheet-orientation-vertical", true).toBool());
    ui->fontColorCheckBox->setChecked(settings.value("use-custom-font-color", true).toBool());
    ui->colorButton->setStyleSheet(QString("QPushButton { background-color : %1; border-style: inset;}")
                                           .arg(settings.value("font-color", "#0097ff").toString()));

    settings.endGroup();

    setSheetSize((int)SheetSize::Custom); //needs to set radioButtons values correctly
}

void PreferencesDialog::setSheetSize(int size)
{
    if (changedByProgram) //dont use this function if spinBoxes values
        return;           //are changed by program, not by user

    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    bool isVertical = settings.value("is-sheet-orientation-vertical", true).toBool();
    settings.endGroup();

    int height = 0;
    int width = 0;

    SheetSize s = SheetSize(size);

    switch (s)
    {
        case SheetSize::A5:
        {
            height = 210;
            width = 148;
            break;
        }
        case SheetSize::A4:
        {
            height = 297;
            width = 210;
            break;
        }
        case SheetSize::Custom:
        {
            //first, check if values fit A4 or A5;
            //in this case set appropriate radioButton checked
            height = ui->sheetHeightSpinBox->value();
            width = ui->sheetWidthSpinBox->value();

            if (!isVertical)
                qSwap(height, width);

            if (height == 210 && width == 148)
            {
                ui->A5RadioButton->setChecked(true);
                return;
            }
            if (height == 297 && width == 210)
            {
                ui->A4RadioButton->setChecked(true);
                return;
            }

            //if they dont fit, set CustomRadioButton checked
            ui->CustomRadioButton->setChecked(true);
            return;
        }
    }

    //set values if A4 or A5 is selected
    if (!isVertical)
        qSwap(height, width);

    changedByProgram = true;
    ui->sheetHeightSpinBox->setValue(height);
    ui->sheetWidthSpinBox->setValue(width);
    changedByProgram = false;
}

void PreferencesDialog::changeSheetOrientation()
{
    changedByProgram = true;
    int temp = ui->sheetHeightSpinBox->value();
    ui->sheetHeightSpinBox->setValue(ui->sheetWidthSpinBox->value());
    ui->sheetWidthSpinBox->setValue(temp);
    changedByProgram = false;
}

void PreferencesDialog::setColor()
{
    QColor currentColor = ui->colorButton->palette().brush(QPalette::Window).color();
    QColor newColor = QColorDialog::getColor(currentColor);

    if (newColor.isValid())
        ui->colorButton->setStyleSheet(QString("QPushButton { background-color : %1; border-style: inset;}").arg(newColor.name()));
}
