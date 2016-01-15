#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QApplication>
#include <qmath.h>
#include <QTransform>

class SvgView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit SvgView(QWidget *parent = 0);

signals:

public slots:

void wheelEvent(QWheelEvent *event);

private:
    qreal maxZoomFactor;
    qreal minZoomFactor;
    qreal scaleFactor;
    void limitScale(qreal factor);
};

#endif // SVGVIEW_H
