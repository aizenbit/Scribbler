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

    loadSettingsFromFile();
    loadSettingsToFile();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::loadSettingsToFile()
{
    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    settings.setValue("dpi", QVariant(ui->dpiSpinBox->value()));
    settings.setValue("letter-spacing", QVariant(ui->letterSpacingSpinBox->value()));
    settings.setValue("font-size", QVariant(ui->fontSizeSpinBox->value()));
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
    settings.endGroup();
}
