#ifndef SVGDATAEDITOR_H
#define SVGDATAEDITOR_H

#include <QtCore/QtMath>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsItem>
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <QtGui/QWheelEvent>

#include "svgview.h"

class SymbolDataEditor : public QGraphicsView
{
    Q_OBJECT

public:
    SymbolDataEditor(QWidget *parent = 0);
    ~SymbolDataEditor();

    void load(const QString & fileName);
    void setSymbolData(const QPointF _inPoint, const QPointF _outPoint, const QRectF _limits);

protected:
    void wheelEvent(QWheelEvent *event);

private:
    enum Item : int {
        SceneRect,
        SymbolItem,
        InPoint,
        OutPoint,
        LimitsRect
    };

    QGraphicsScene *scene;
    qreal maxScaleFactor, minScaleFactor, currentScaleFactor;
    QPointF inPoint, outPoint;
    QRectF limits;
    void limitScale(qreal factor);  //limited view zoom
    QPointF toStored(const QPointF &point);
    QPointF fromStored(const QPointF &point);
};

#endif // SVGDATAEDITOR_H
