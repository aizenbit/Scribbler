#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

public slots:
    void loadSettingsToFile();
    void loadSettingsFromFile();

signals:
    void settingsChanged();

private:
    Ui::PreferencesDialog *ui;
};

#endif // PREFERENCESDIALOG_H
