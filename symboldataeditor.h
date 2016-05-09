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
    void disableChanges();
    QPointF getInPoint() const {return toStored(inPoint);}
    QPointF getOutPoint() const {return toStored(outPoint);}
    QRectF getLimits() const {return QRectF(toStored(limits.topLeft()),
                                            toStored(limits.bottomRight()));}

protected:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    enum Item : int {
        SceneRect,
        SymbolItem,
        InPoint,
        OutPoint,
        LimitsRect,
        NoItem
    };

    QGraphicsScene *scene;
    qreal maxScaleFactor, minScaleFactor, currentScaleFactor;
    Item itemToChange;
    //bool changeInPoint, changeOutPoint, changeLimits;
    qreal pointWidth;
    QPointF inPoint, outPoint;
    QRectF limits;

    void limitScale(qreal factor);  //limited view zoom
    QPointF toStored(const QPointF &point) const;
    QPointF fromStored(const QPointF &point) const;
    void moveItem(const QPoint pos);
};

#endif // SVGDATAEDITOR_H
