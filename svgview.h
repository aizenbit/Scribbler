#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QApplication>
#include <qmath.h>
#include <QGraphicsSvgItem>

class SvgView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit SvgView(QWidget *parent = 0);
    ~SvgView();

signals:

public slots:
    void renderText(QString text);

    void wheelEvent(QWheelEvent *event);

private:
    qreal maxZoomFactor;
    qreal minZoomFactor;
    qreal scaleFactor;
    void limitScale(qreal factor);

    QGraphicsScene * scene;
    int dpi;
    int dpmm; //dots per mm
    QRectF sheetRect, marginsRect;

    QMultiMap<QChar, QString> font;
    qreal fontSize;


};

#endif // SVGVIEW_H
