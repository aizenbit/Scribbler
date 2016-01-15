#include "svgview.h"

SvgView::SvgView(QWidget *parent) : QGraphicsView(parent)
{
    scaleFactor = 1.0;
    maxZoomFactor = 2.0;
    minZoomFactor = 0.1;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    limitScale(0.3);

    dpi = 300;
    dpmm = dpi / 25.4;
    sheetRect = QRectF(0, 0, 210 * dpmm, 297 * dpmm);
    marginsRect = QRectF(5 * dpmm, 5 * dpmm, 200 * dpmm, 287 * dpmm);
    fontSize = 6.0;

    scene = new QGraphicsScene(sheetRect);
    scene->addRect(sheetRect);
    scene->addRect(marginsRect, QPen(Qt::darkGray));

    //for testing
    font.insert('a', "E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_A.svg");
    font.insert('a', "E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_A2.svg");
    font.insert('b', "E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_B.svg");
    font.insert('c', "E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_C.svg");

    scene->addItem(new QGraphicsSvgItem(font.value('a')));
    setScene(scene);
}

SvgView::~SvgView()
{
    delete scene;
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

void SvgView::renderText(QString text)
{
    scene->clear();
    QPointF cursor(10.0,10.0);
    for (QChar symbol : text)
    {
        cursor += QPointF(cursor.x() > (sheetRect.x() + sheetRect.width()) ? 10.0 - cursor.x() : 25.0,
                          cursor.x() > (sheetRect.x() + sheetRect.width()) ? 40.0              : 0.0);

        if (!font.contains(symbol))
            continue;

        QGraphicsSvgItem * letter = new QGraphicsSvgItem(font.values(symbol).at(qrand() % font.values(symbol).size()));
        letter->setPos(cursor);
        scene->addItem(letter);
    }
}
