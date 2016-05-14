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
    explicit SymbolDataEditor(QWidget *parent = 0);
    ~SymbolDataEditor();

    void load(const QString & fileName);
    void setSymbolData(const QPointF _inPoint, const QPointF _outPoint, const QRectF _limits);
    void clear();
    void disableChanges();
    QPointF getInPoint() const {return toStored(inPoint);}
    QPointF getOutPoint() const {return toStored(outPoint);}
    QRectF getLimits() const {return QRectF(toStored(limits.topLeft()),
                                            toStored(limits.bottomRight()));}

public slots:
    void enableInPointChanges();
    void enableOutPointChanges();
    void enableLimitsChanges();

protected:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent * event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private:
    enum Item : int {
        SymbolItem,
        InPoint,
        OutPoint,
        LimitsRect,
        NoItem
    };

    enum Side : int {
        NoSide = 0x00,
        Top = 0x01,
        Bottom = 0x02,
        Left = 0x04,
        Right = 0x08,
        AllSides = 0x10
    };
    const qreal maxScaleFactor = 40, minScaleFactor = 0.1, sceneScale = 5;
    QGraphicsScene *scene;
    qreal currentScaleFactor;
    Item itemToChange;
    Side sideToChange; //for LimitsRect
    qreal pointWidth;
    QPointF inPoint, outPoint, dLimitsCenter;
    QRectF limits;
    QDomDocument doc;

    void limitScale(qreal factor);  //limited view zoom
    QPointF toStored(const QPointF &point) const;
    QPointF fromStored(const QPointF &point) const;
    QPointF fromViewBox(const QPointF &point) const;
    void moveItem(const QPoint pos);
    void calculateSideToChange(QPoint pos);
    void changeCursor();
    void rememberChanges();
    void correctLimits();
    void addDataItems();
    QPointF getBeginPoint();
    QPointF getTranslatePoint();
    QPointF getMovePoint(const QString &path);
    QStringList getPathList();
};

#endif // SVGDATAEDITOR_H
