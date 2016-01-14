#include "svgview.h"

SvgView::SvgView(QWidget *parent) : QGraphicsView(parent)
{
    scaleFactor = 1.0;
    maxZoomFactor = 2.0;
    minZoomFactor = 0.1;

    setDragMode(ScrollHandDrag);
    limitScale(0.3);
}

void SvgView::wheelEvent(QWheelEvent *event)
{
    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        zoom(event);
        event->accept();
    }
}

void SvgView::zoom(QWheelEvent *event)
{
    QPointF target_scene_pos = mapToScene(event->pos());
    qreal factor = qPow(1.2, event->delta() / 240.0);

    limitScale(factor);

    QPointF delta_viewport_pos = event->pos() - QPointF(viewport()->width() / 2.0,
                                                        viewport()->height() / 2.0);
    QPointF viewport_center = mapFromScene(target_scene_pos) - delta_viewport_pos;
    centerOn(mapToScene(viewport_center.toPoint()));
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
