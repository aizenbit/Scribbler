#ifndef SVGEDITOR_H
#define SVGEDITOR_H

#include <QtSvg/QSvgWidget>
#include <QtSvg/QSvgRenderer>

class SvgEditor : public QSvgWidget
{
    Q_OBJECT
public:
    explicit SvgEditor(QWidget *parent = 0);
    void load(const QString & file);

signals:

public slots:
};

#endif // SVGEDITOR_H
