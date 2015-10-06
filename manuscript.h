#ifndef MANUSCRIPT_H
#define MANUSCRIPT_H

#include <QGraphicsScene>


class Manuscript : public  QGraphicsScene
{
public:
    QSize sheetSize;
    QSize margins;
    QSize indent;
    Manuscript(QObject * parent = 0);
    ~Manuscript();

    void newTemplate();
};

#endif // MANUSCRIPT_H
