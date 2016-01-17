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
            this, SLOT(loadSettingsToFile()));

    sheetSizeSignalMapper = new QSignalMapper(this);

    connect(ui->A4RadioButton, SIGNAL(clicked()),
            sheetSizeSignalMapper, SLOT(map()));
    connect(ui->A4HRadioButton, SIGNAL(clicked()),
            sheetSizeSignalMapper, SLOT(map()));
    connect(ui->A5RadioButton, SIGNAL(clicked()),
            sheetSizeSignalMapper, SLOT(map()));
    connect(ui->sheetHeightSpinBox, SIGNAL(valueChanged(int)),
            sheetSizeSignalMapper, SLOT(map()));
    connect(ui->sheetWidthSpinBox, SIGNAL(valueChanged(int)),
            sheetSizeSignalMapper, SLOT(map()));

    sheetSizeSignalMapper->setMapping(ui->A4RadioButton, (int)SheetSize::A4);
    sheetSizeSignalMapper->setMapping(ui->A4HRadioButton, (int)SheetSize::A4H);
    sheetSizeSignalMapper->setMapping(ui->A5RadioButton, (int)SheetSize::A5);
    sheetSizeSignalMapper->setMapping(ui->sheetHeightSpinBox, (int)SheetSize::Custom);
    sheetSizeSignalMapper->setMapping(ui->sheetWidthSpinBox, (int)SheetSize::Custom);

    connect (sheetSizeSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(setSheetSize(int)));

    loadSettingsFromFile();
    loadSettingsToFile();

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
    settings.setValue("font-size", QVariant(ui->fontSizeSpinBox->value()));
    settings.setValue("sheet-width",QVariant(ui->sheetWidthSpinBox->value()));
    settings.setValue("sheet-height",QVariant(ui->sheetHeightSpinBox->value()));
    settings.setValue("right-margin",QVariant(ui->rightMarginsSpinBox->value()));
    settings.setValue("left-margin",QVariant(ui->leftMarginsSpinBox->value()));
    settings.setValue("top-margin",QVariant(ui->topMarginsSpinBox->value()));
    settings.setValue("bottom-margin",QVariant(ui->bottomMarginsSpinBox->value()));
    settings.endGroup();

    emit settingsChanged();
}

void PreferencesDialog::loadSettingsFromFile()
{
    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    ui->dpiSpinBox->setValue(settings.value("dpi", 300).toInt());
    ui->letterSpacingSpinBox->setValue(settings.value("letter-spacing", -10.0).toDouble());
    ui->fontSizeSpinBox->setValue(settings.value("font-size", 6.0).toDouble());
    ui->sheetWidthSpinBox->setValue(settings.value("sheet-width", 210.0).toInt());
    ui->sheetHeightSpinBox->setValue(settings.value("sheet-height", 297.0).toInt());
    ui->rightMarginsSpinBox->setValue(settings.value("right-margin", 5.0).toInt());
    ui->leftMarginsSpinBox->setValue(settings.value("left-margin", 5.0).toInt());
    ui->topMarginsSpinBox->setValue(settings.value("top-margin", 5.0).toInt());
    ui->bottomMarginsSpinBox->setValue(settings.value("bottom-margin", 5.0).toInt());
    settings.endGroup();
}

void PreferencesDialog::setSheetSize(int size)
{
    SheetSize s = SheetSize(size);

    switch (s)
    {
        case SheetSize::A5:
        {
            changedByProgram = true;
            ui->sheetHeightSpinBox->setValue(148);
            ui->sheetWidthSpinBox->setValue(210);
            changedByProgram = false;
            break;
        }
        case SheetSize::A4:
        {
            changedByProgram = true;
            ui->sheetHeightSpinBox->setValue(297);
            ui->sheetWidthSpinBox->setValue(210);
            changedByProgram = false;
            break;
        }
        case SheetSize::A4H:
        {
            changedByProgram = true;
            ui->sheetHeightSpinBox->setValue(210);
            ui->sheetWidthSpinBox->setValue(297);
            changedByProgram = false;
            break;
        }
        case SheetSize::Custom:
        {
            if (changedByProgram)
                return;
            int height = ui->sheetHeightSpinBox->value();
            int width = ui->sheetWidthSpinBox->value();
            if ((height == 148 && width == 210) ||
                (height == 210 && width == 297) ||
                (height == 297 && width == 210))
                return;

            ui->CustomRadioButton->setChecked(true);
            break;
        }
    }
}
