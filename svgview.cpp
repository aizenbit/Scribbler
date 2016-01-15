#include "svgview.h"

SvgView::SvgView(QWidget *parent) : QGraphicsView(parent)
{
    scaleFactor = 1.0;
    maxZoomFactor = 2.0;
    minZoomFactor = 0.1;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    limitScale(0.3);
}

void SvgView::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->delta() / 240.0);
    limitScale(factor);
    event->accept();
}

void SvgView::limitScale(qreal factor)
{
    qreal newFactor = scaleFactor * factor;
    if (newFactor < maxZoomFactor && newFactor > minZoomFactor)
    {
        scaleFactor = newFactor;
        scale(factor, factor);
    }
}
