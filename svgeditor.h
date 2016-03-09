#ifndef SVGEDITOR_H
#define SVGEDITOR_H

#include <QtCore/QtMath>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtSvg/QSvgWidget>
#include <QtSvg/QSvgRenderer>

#include "svgview.h"

class SvgEditor : public QSvgWidget
{
    Q_OBJECT

public:
    explicit SvgEditor(QWidget *parent = 0);

public slots:
    void load(const QString & file);
    void setSymbolData(const QPointF _inPoint, const QPointF _outPoint, const QRectF _limits);
    void disableDrawing(const bool disable = true);
    void hideAll(const bool hide = true);
    void enableInPointDrawing(const bool draw = true);
    void enableOutPointDrawing(const bool draw = true);
    void enableLimitsDrawing(const bool draw = true);
    QPointF getInPoint() const {return inPoint;}
    QPointF getOutPoint() const {return outPoint;}
    QRectF getLimits() const {return limits;}

protected:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QPointF inPoint, outPoint, limitsTopLeft, limitsBottomRight, leftCornerPos;
    QRectF limits;
    bool showInPoint, showOutPoint, showLimits, showSymbol;
    bool drawInPoint, drawOutPoint, drawLimits, drawSymbol;
    const qreal pointWidth = 5;
    qreal scaleFactor, maxScaleFactor, minScaleFactor;
    QSize currentSymbolSize;
    QPointF symbolBegin;

    void setInPoint(QPointF point);
    void setOutPoint(QPointF point);
    void setLimitsTopLeft(QPointF point);
    void setLimitsBottomRight(QPointF point);
    void calculateCoordinates();
    void keepPointOnSymbolCanvas(QPointF &point);
    QPointF toStored(const QPointF &point);
    QPointF fromStored(const QPointF &point);
    void scaleSVGCanvas(QString fileName);
};

#endif // SVGEDITOR_H
