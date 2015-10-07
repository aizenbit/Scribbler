#ifndef MANUSCRIPT_H
#define MANUSCRIPT_H

#include <QGraphicsScene>
#include <QWheelEvent>

class Manuscript : public  QGraphicsScene
{
    Q_OBJECT

public:
    QSize sheetSize;
    QSize margins;
    QSize indent;
    int dpi;
    int dpmm; //dots per mm
    Manuscript(QObject * parent = 0);
    ~Manuscript();

    void newTemplate();

public slots:

signals:

};

#endif // MANUSCRIPT_H
