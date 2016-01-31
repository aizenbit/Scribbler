#ifndef SVGEDITOR_H
#define SVGEDITOR_H

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtSvg/QSvgWidget>
#include <QtSvg/QSvgRenderer>

class SvgEditor : public QSvgWidget
{
    Q_OBJECT
public:
    explicit SvgEditor(QWidget *parent = 0);

    bool drawInPoint, drawOutPoint, drawLimits, drawLetter;

public slots:
    void load(const QString & file);
    void setLetterData(const QPointF _inPoint, const QPointF _outPoint, const QRectF _limits);
    void disableDrawing(bool disable = true);
    void enableInPointDrawing(bool draw = true);
    void enableOutPointDrawing(bool draw = true);
    void enableLimitsDrawing(bool draw = true);
    QPointF getInPoint() const {return inPoint;}
    QPointF getOutPoint() const {return outPoint;}
    QRectF getLimits() const {return limits;}

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    QPointF inPoint, outPoint, limitsTopLeft, limitsBottomRight;
    QRectF limits;
    bool showInPoint, showOutPoint, showLimits;
    const qreal pointWidth = 5;
    QSize letterSize;

    void setInPoint(const QPointF &point);
    void setOutPoint(const QPointF &point);
    void setLimitsTopLeft(const QPointF &point);
    void setLimitsBottomRight(const QPointF &point);
    QPointF toStored(const QPointF &point);
    QPointF fromStored(const QPointF &point);
};

#endif // SVGEDITOR_H
