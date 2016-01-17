#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QSignalMapper>

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
    QSignalMapper * sheetSizeSignalMapper;

    enum class SheetSize{
        A4,
        A4H,
        A5,
        Custom
    };

    bool changedByProgram;

private slots:
    void setSheetSize(int size);

};

#endif // PREFERENCESDIALOG_H
