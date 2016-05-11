#include "symboldataeditor.h"

SymbolDataEditor::SymbolDataEditor(QWidget *parent) : QGraphicsView(parent)
{
    currentScaleFactor = 1.0;
    minScaleFactor = 0.1;
    maxScaleFactor = 20;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::NoDrag);
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
    setDragMode(QGraphicsView::ScrollHandDrag);
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
    qDebug() << _inPoint << _outPoint << toStored(inPoint) << toStored(outPoint) << "\n";
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
    result -= QPointF(1.0, 1.0);
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
    setDragMode(QGraphicsView::NoDrag);
}

void SymbolDataEditor::disableChanges()
{
    itemToChange = Item::NoItem;
    sideToChange = NoSide;
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
    sideToChange = NoSide;
}

void SymbolDataEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton)
    {
        QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fake);
        QApplication::restoreOverrideCursor();
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
    if (event->button() != Qt::MidButton && !(event->buttons() & Qt::LeftButton) &&
            itemToChange == Item::LimitsRect)
    {
        calculateSideToChange(event->pos());
        changeCursor();
    }

    if (event->buttons() & Qt::LeftButton)
        moveItem(event->pos());

    QGraphicsView::mouseMoveEvent(event);
}

void SymbolDataEditor::mouseReleaseEvent(QMouseEvent *event)
{
    rememberChanges();

    if (event->button() == Qt::MidButton)
    {
        QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&fake);
        QApplication::setOverrideCursor(Qt::ArrowCursor);
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

void SymbolDataEditor::calculateSideToChange(QPoint pos)
{
    sideToChange = Side::NoSide;

    if (itemToChange != Item::LimitsRect)
        return;

    QGraphicsRectItem* limitsItem = static_cast<QGraphicsRectItem *>(scene->items(Qt::AscendingOrder).at(itemToChange));
    QRectF limitsRect = limitsItem->rect();

    QPoint topLeft = mapFromScene(limitsRect.topLeft());
    QPoint bottomRight = mapFromScene(limitsRect.bottomRight());

    if (abs(topLeft.x() - pos.x()) < 10)
        sideToChange = static_cast<Side>(sideToChange | Side::Left);
    if (abs(topLeft.y() - pos.y()) < 10)
        sideToChange = static_cast<Side>(sideToChange | Side::Top);
    if (abs(bottomRight.x() - pos.x()) < 10)
        sideToChange = static_cast<Side>(sideToChange | Side::Right);
    if (abs(bottomRight.y() - pos.y()) < 10)
        sideToChange = static_cast<Side>(sideToChange | Side::Bottom);

    if (sideToChange & Side::Left && sideToChange & Side::Right)
        sideToChange = Side::Left;
    if (sideToChange & Side::Top && sideToChange & Side::Bottom)
        sideToChange = Side::Top;

    if (sideToChange == Side::NoSide && limitsRect.contains(mapToScene(pos)))
    {
        sideToChange = Side::AllSides;
        dLimitsCenter = limitsRect.center() - mapToScene(pos);
    }
}

void SymbolDataEditor::changeCursor()
{
    if (QApplication::overrideCursor()->shape() != Qt::ArrowCursor)
        QApplication::restoreOverrideCursor();

    if (sideToChange == Side::Left || sideToChange == Side::Right)
        QApplication::setOverrideCursor(Qt::SizeHorCursor);
    if (sideToChange == Side::Top || sideToChange == Side::Bottom)
        QApplication::setOverrideCursor(Qt::SizeVerCursor);

    if (sideToChange == (Side::Left | Side::Top) ||
            sideToChange == (Side::Right | Side::Bottom))
        QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
    if (sideToChange == (Side::Left | Side::Bottom) ||
            sideToChange == (Side::Right | Side::Top))
        QApplication::setOverrideCursor(Qt::SizeBDiagCursor);

    if (sideToChange == Side::AllSides)
        QApplication::setOverrideCursor(Qt::SizeAllCursor);

}

void SymbolDataEditor::moveItem(const QPoint pos)
{
    if (itemToChange == Item::NoItem)
        return;

    if (itemToChange == Item::InPoint || itemToChange == Item::OutPoint)
    {
        QPointF itemPos = mapToScene(pos);
        QGraphicsEllipseItem* item = static_cast<QGraphicsEllipseItem *>(scene->items(Qt::AscendingOrder).at(itemToChange));
        item->setRect(itemPos.x() - pointWidth / 2,
                      itemPos.y() - pointWidth / 2,
                      pointWidth, pointWidth);
    }

    if (itemToChange == Item::LimitsRect)
    {
        QGraphicsRectItem* item = static_cast<QGraphicsRectItem *>(scene->items(Qt::AscendingOrder).at(itemToChange));

        QPointF scenePos = mapToScene(pos);
        QRectF newRect = item->rect();

        if (sideToChange & Side::Top)
            newRect.setTop(scenePos.y());
        if (sideToChange & Side::Bottom)
            newRect.setBottom(scenePos.y());
        if (sideToChange & Side::Left)
            newRect.setLeft(scenePos.x());
        if (sideToChange & Side::Right)
            newRect.setRight(scenePos.x());

        if (sideToChange == Side::AllSides)
        {
            newRect.moveCenter(scenePos + dLimitsCenter);
        }

        item->setRect(newRect);
    }
}

void SymbolDataEditor::rememberChanges()
{
    if (scene->items().isEmpty())
        return;

    if (itemToChange == Item::InPoint)
    {
        QGraphicsEllipseItem* inPointItem = static_cast<QGraphicsEllipseItem *>(scene->items(Qt::AscendingOrder).at(Item::InPoint));
        inPoint = inPointItem->rect().center();
    }

    if (itemToChange == Item::OutPoint)
    {
        QGraphicsEllipseItem* outPointItem = static_cast<QGraphicsEllipseItem *>(scene->items(Qt::AscendingOrder).at(Item::OutPoint));
        outPoint = outPointItem->rect().center();
    }

    if (itemToChange == Item::LimitsRect)
    {
        QGraphicsRectItem* limitsRectItem = static_cast<QGraphicsRectItem *>(scene->items(Qt::AscendingOrder).at(Item::LimitsRect));
        limits = limitsRectItem->rect();
    }
}
