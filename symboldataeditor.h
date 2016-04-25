#ifndef SVGDATAEDITOR_H
#define SVGDATAEDITOR_H

#include <QtCore/QtMath>
#include <QtWidgets/QGraphicsView>
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <QtGui/QWheelEvent>

class SymbolDataEditor : public QGraphicsView
{
    Q_OBJECT

public:
    SymbolDataEditor(QWidget *parent = 0);
    ~SymbolDataEditor();

     void load(const QString & file);

protected:
    void wheelEvent(QWheelEvent *event);

private:
     QGraphicsScene *scene;
     qreal maxScaleFactor, minScaleFactor, currentScaleFactor;

void limitScale(qreal factor);  //limited view zoom
};

#endif // SVGDATAEDITOR_H
