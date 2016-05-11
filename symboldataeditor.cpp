#include "symboldataeditor.h"

SymbolDataEditor::SymbolDataEditor(QWidget *parent) : QGraphicsView(parent)
{
    currentScaleFactor = 1.0;
    minScaleFactor = 0.1;
    maxScaleFactor = 20;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::ScrollHandDrag);
    disableChanges();
    setRenderHints(QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);

    scene = new QGraphicsScene();
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
    clear();
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
    pointWidth = maxSceneSide / 1000;
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

void SymbolDataEditor::clear()
{
    scene->clear();
    inPoint = QPointF();
    outPoint = QPointF();
    limits = QRectF();
}

void SymbolDataEditor::disableChanges()
{
    itemToChange = Item::NoItem;
}

void SymbolDataEditor::enableInPointChanges()
{
    itemToChange = Item::InPoint;
}

void SymbolDataEditor::enableOutPointChanges()
{
    itemToChange = Item::OutPoint;
}

void SymbolDataEditor::enableLimitsChanges()
{
    itemToChange = Item::LimitsRect;
}

void SymbolDataEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton)
    {
        QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fake);
        return;
    }

    if (event->button() == Qt::LeftButton)
    {
        moveItem(event->pos());
        QMouseEvent fake(event->type(), event->pos(), Qt::MidButton, Qt::MidButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fake);
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void SymbolDataEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier) && event->buttons() == Qt::LeftButton)
        moveItem(event->pos());

    QGraphicsView::mouseMoveEvent(event);
}

void SymbolDataEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton)
    {
        QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&fake);
    }
    else
        QGraphicsView::mouseReleaseEvent(event);
}

void SymbolDataEditor::enterEvent(QEvent *event)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    QGraphicsView::enterEvent(event);
}

void SymbolDataEditor::leaveEvent(QEvent *event)
{
    QApplication::restoreOverrideCursor();
    QGraphicsView::enterEvent(event);
}

void SymbolDataEditor::moveItem(const QPoint pos)
{
    if (itemToChange == Item::NoItem)
        return;

    QPointF itemPos = mapToScene(pos);

    if (itemToChange == Item::InPoint || itemToChange == Item::OutPoint)
    {
        QGraphicsEllipseItem* item = static_cast<QGraphicsEllipseItem*>(scene->items(Qt::AscendingOrder).at(itemToChange));
        item->setRect(itemPos.x() - pointWidth / 2,
                      itemPos.y() - pointWidth / 2,
                      pointWidth, pointWidth);
    }

    if (itemToChange == Item::LimitsRect)
    {
        QGraphicsRectItem* item = static_cast<QGraphicsRectItem*>(scene->items(Qt::AscendingOrder).at(itemToChange));
        QPoint globalCursorPos = QCursor::pos();
        QPoint viewPos = mapFromGlobal(globalCursorPos);
        QPointF scenePos = mapToScene(viewPos);
        QRectF newRect = item->rect();
        newRect.moveCenter(scenePos);

        qDebug() << pos << globalCursorPos << viewPos << scenePos << "\n";

        if (item->rect().contains(scenePos))
            item->setRect(newRect);
    }
}
