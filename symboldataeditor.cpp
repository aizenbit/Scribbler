#include "symboldataeditor.h"

SymbolDataEditor::SymbolDataEditor(QWidget *parent) : QGraphicsView(parent)
{
    currentScaleFactor = 1.0;

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
    doc = QDomDocument("SVG");
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
    scene->setSceneRect(0, 0, itemSize.width() * sceneScale, itemSize.height() * sceneScale);
    symbolItem->setPos(scene->width() / 2 - itemSize.width() / 2.0,
                       scene->height() / 2 - itemSize.height() / 2.0);
    scene->addItem(symbolItem);
    limitScale(qMax(qreal(width()) / itemSize.width(),
                    qreal(height()) / itemSize.height()) / currentScaleFactor);
    centerOn(symbolItem);
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
    QRectF symbolRect = scene->items(Qt::AscendingOrder).at(Item::SymbolItem)->boundingRect();
    symbolRect.moveTopLeft(scene->items(Qt::AscendingOrder).at(Item::SymbolItem)->pos());
    symbolRect.adjust(symbolRect.width() / 4, symbolRect.height() / 4,
                      -symbolRect.width() / 4, -symbolRect.height() / 4);

    if (_inPoint.isNull())
    {
        inPoint = getBeginPoint();
        if (inPoint.isNull())
        {
            inPoint = symbolRect.topLeft();
            inPoint.ry() += symbolRect.height() / 2;
        }
    }
    else
        inPoint = fromStored(_inPoint);

    if (_outPoint.isNull())
    {
        outPoint = symbolRect.topRight();
        outPoint.ry() += symbolRect.height() / 2;
    }
    else
        outPoint = fromStored(_outPoint);

    if (_limits.isNull())
        limits = symbolRect;
    else
        limits = QRectF(fromStored(_limits.topLeft()),
                        fromStored(_limits.bottomRight()));

    correctLimits();
    addDataItems();
}

QPointF SymbolDataEditor::toStored(const QPointF &point) const
{
    QRectF symbolRect = scene->items(Qt::AscendingOrder).at(Item::SymbolItem)->boundingRect();
    symbolRect.moveTopLeft(scene->items(Qt::AscendingOrder).at(Item::SymbolItem)->pos());
    QPointF result = point - symbolRect.topLeft();
    result.rx() = point.x() / symbolRect.width();
    result.ry() = point.y() / symbolRect.height();
    result -= QPointF(2.0,2.0); //sorry, i don't know why this works, but it depends on scaleCanvas
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

QPointF SymbolDataEditor::fromViewBox(const QPointF &point) const
{
    QPointF result = point;
    QDomElement svgElement = doc.elementsByTagName("svg").item(0).toElement();
    QStringList viewBoxValues = svgElement.attribute("viewBox").split(" ");

    QRectF viewBox = QRectF(viewBoxValues.at(0).toDouble(), viewBoxValues.at(1).toDouble(),
                            viewBoxValues.at(2).toDouble(), viewBoxValues.at(3).toDouble());

    result -= viewBox.topLeft();
    result.rx() /= viewBox.width();
    result.ry() /= viewBox.height();
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

        while (QApplication::overrideCursor() != nullptr)
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
    if (!(event->buttons() & Qt::MidButton) && !(event->buttons() & Qt::LeftButton))
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

        if (QApplication::overrideCursor() == nullptr)
            changeCursor();
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
    while(QApplication::overrideCursor() != nullptr)
        QApplication::restoreOverrideCursor();

    QGraphicsView::leaveEvent(event);
}

void SymbolDataEditor::calculateSideToChange(QPoint pos)
{
    sideToChange = Side::NoSide;

    if (itemToChange != Item::LimitsRect || scene->items().isEmpty())
        return;

    QGraphicsRectItem* limitsItem = static_cast<QGraphicsRectItem *>(scene->items(Qt::AscendingOrder).at(itemToChange));
    QRectF limitsRect = limitsItem->rect();

    QPoint topLeft = mapFromScene(limitsRect.topLeft());
    QPoint bottomRight = mapFromScene(limitsRect.bottomRight());

    if (QRectF(topLeft, bottomRight).adjusted(-10,-10,10,10).contains(pos))
    {
        if (abs(topLeft.x() - pos.x()) < 10)
            sideToChange = static_cast<Side>(sideToChange | Side::Left);
        if (abs(topLeft.y() - pos.y()) < 10)
            sideToChange = static_cast<Side>(sideToChange | Side::Top);
        if (abs(bottomRight.x() - pos.x()) < 10)
            sideToChange = static_cast<Side>(sideToChange | Side::Right);
        if (abs(bottomRight.y() - pos.y()) < 10)
            sideToChange = static_cast<Side>(sideToChange | Side::Bottom);
    }

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
    if (QApplication::overrideCursor() != nullptr &&
            QApplication::overrideCursor()->shape() != Qt::ArrowCursor)
        QApplication::restoreOverrideCursor();

    if (itemToChange == Item::LimitsRect)
    {
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

    if (itemToChange == Item::InPoint || itemToChange == Item::OutPoint)
        QApplication::setOverrideCursor(Qt::PointingHandCursor);

    if (QApplication::overrideCursor() == nullptr)
        QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void SymbolDataEditor::moveItem(const QPoint pos)
{
    if (itemToChange == Item::NoItem || scene->items().isEmpty())
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

    correctLimits();
}

void SymbolDataEditor::correctLimits()
{
    if (limits.height() < 0)
    {
        qreal top = limits.top();
        limits.setTop(limits.bottom());
        limits.setBottom(top);
    }

    if (limits.width() < 0)
    {
        qreal left = limits.left();
        limits.setLeft(limits.right());
        limits.setRight(left);
    }

    if (scene->items().size() > Item::LimitsRect)
    {
        QGraphicsRectItem* limitsRectItem = static_cast<QGraphicsRectItem *>(scene->items(Qt::AscendingOrder).at(Item::LimitsRect));
        limitsRectItem->setRect(limits);
    }

}

void SymbolDataEditor::addDataItems()
{
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

QPointF SymbolDataEditor::getBeginPoint()
{
    QPointF result = QPointF(0,0);
    QDomElement gElement = doc.elementsByTagName("g").item(0).toElement();
    QString transform = gElement.attribute("transform");

    if (transform.contains("translate"))
    {
        QString translate = QRegularExpression("translate(.+)").match(transform).captured();
        translate.remove("translate(", Qt::CaseInsensitive);
        translate.remove(")");
        QString x = translate.left(translate.indexOf(','));
        QString y = translate.mid(translate.indexOf(',') + 1);
        result = QPointF(x.toDouble(), y.toDouble());
    }

    QDomNodeList pathList = doc.elementsByTagName("path");

    if (pathList.isEmpty())
        return QPointF();

    QDomElement pathElement = pathList.item(0).toElement();
    QString path = pathElement.attribute("d");
    QString move = QRegularExpression("^[mM] *-?\\d+\\.?\\d*,? ?-?\\d+\\.?\\d*").match(path).captured();
    bool absolutely = move.at(0).isUpper();
    move.remove(QRegularExpression("^[mM] *"));
    QString x = move.left(move.indexOf(QRegularExpression("[ ,]")));
    QString y = move.mid(move.indexOf(QRegularExpression("[ ,]")) + 1);

    if (absolutely)
        result = QPointF(x.toDouble(), y.toDouble());
    else
        result += QPointF(x.toDouble(), y.toDouble());

    result = fromViewBox(result);

    return fromStored(result);
}
