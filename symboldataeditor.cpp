#include "symboldataeditor.h"

SymbolDataEditor::SymbolDataEditor(QWidget *parent) : QGraphicsView(parent)
{
    currentScaleFactor = 1.0;
    minScaleFactor = 0.1;
    maxScaleFactor = 20;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(NoDrag);
    setRenderHints(QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);

    scene = new QGraphicsScene();
    scene->addRect(0,0,scene->width()-1,scene->height()-1);

    setScene(scene);
}

SymbolDataEditor::~SymbolDataEditor()
{
    delete scene;
}

void SymbolDataEditor::load(const QString & fileName)
{
    QDomDocument doc("SVG");
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }

    file.close();

    QDomElement svgElement = doc.elementsByTagName("svg").item(0).toElement();
    SvgView::scaleViewBox(svgElement);
    QByteArray scaledFile = doc.toString(0).replace(">\n<tspan", "><tspan").toUtf8();

    QGraphicsSvgItem *symbolItem = new QGraphicsSvgItem();
    symbolItem->setSharedRenderer(new QSvgRenderer(scaledFile));

    QSizeF itemSize = symbolItem->renderer()->defaultSize();
    scene->clear();
    scene->setSceneRect(0, 0, itemSize.width() * 3, itemSize.height() * 3);
    symbolItem->setPos(scene->width() / 2 - itemSize.width() / 2.0,
                       scene->height() / 2 - itemSize.height() / 2.0);
    scene->addRect(0, 0, scene->width() - 1, scene->height() - 1);
    scene->addItem(symbolItem);
}

void SymbolDataEditor::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->delta() / 240.0);
    limitScale(factor);
    event->accept();
}

void SymbolDataEditor::limitScale(qreal factor)
{
    qreal newFactor = currentScaleFactor * factor;

    if (newFactor < maxScaleFactor && newFactor > minScaleFactor)
    {
        currentScaleFactor = newFactor;
        scale(factor, factor);
    }
}

void SymbolDataEditor::setSymbolData(const QPointF _inPoint, const QPointF _outPoint, const QRectF _limits)
{
    inPoint = fromStored(_inPoint);
    outPoint = fromStored(_outPoint);
    limits = QRectF(fromStored(_limits.topLeft()),
                    fromStored(_limits.bottomRight()));

    qreal maxSceneSide = qMax(scene->sceneRect().width(), scene->sceneRect().height());
    qreal pointWidth = maxSceneSide / 1000;
    scene->addEllipse(inPoint.x() - pointWidth / 2,
                      inPoint.y() - pointWidth / 2,
                      pointWidth, pointWidth,
                      QPen(Qt::darkCyan), QBrush(Qt::cyan));
    scene->addEllipse(outPoint.x() - pointWidth / 2,
                      outPoint.y() - pointWidth / 2,
                      pointWidth, pointWidth,
                      QPen(Qt::darkMagenta), QBrush(Qt::magenta));
    scene->addRect(limits, QPen(Qt::darkYellow, 0));
}

QPointF SymbolDataEditor::toStored(const QPointF &point) const
{
    QRectF symbolRect = scene->items(Qt::AscendingOrder).at(Item::SymbolItem)->boundingRect();
    symbolRect.moveTopLeft(scene->items(Qt::AscendingOrder).at(Item::SymbolItem)->pos());
    QPointF result = point - symbolRect.topLeft();
    result.rx() = point.x() / symbolRect.width();
    result.ry() = point.y() / symbolRect.height();
    return result;
}

QPointF SymbolDataEditor::fromStored(const QPointF &point) const
{
    QPointF result;
    QRectF symbolRect = scene->items(Qt::AscendingOrder).at(Item::SymbolItem)->boundingRect();
    symbolRect.moveTopLeft(scene->items(Qt::AscendingOrder).at(Item::SymbolItem)->pos());
    result.rx() = point.x() * symbolRect.width();
    result.ry() = point.y() * symbolRect.height();
    result += symbolRect.topLeft();
    return result;
}
