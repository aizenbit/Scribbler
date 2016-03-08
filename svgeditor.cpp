#include "svgeditor.h"

SvgEditor::SvgEditor(QWidget *parent) : QSvgWidget(parent)
{
    QPalette svgPalette = this->palette();
    svgPalette.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(svgPalette);
    inPoint = QPointF(-1.0, -1.0);
    outPoint = QPointF(-1.0, -1.0);
    limits = QRectF(-1.0,-1.0,-1.0,-1.0);
    leftCornerPos = QPointF(0.0, 0.0);
    scaleFactor = 1;
    minScaleFactor = 0.1;
    maxScaleFactor = 20;
    limitsTopLeft = limits.topLeft();
    limitsBottomRight = limits.bottomRight();
    disableDrawing();
    hideAll();
}

void SvgEditor::load(const QString & file)
{
    QSvgWidget::load(file);
    scaleSVGCanvas(file);
    calculateCoordinates();
    QPointF letterEnd(width() / 2.0 + renderer()->defaultSize().width() / 2.0 * scaleFactor,
                      height() / 2.0 + renderer()->defaultSize().height() / 2.0 * scaleFactor);
    inPoint = toStored(QPointF(letterBegin.x(), letterEnd.y()));
    inPoint.ry() /= 2;
    outPoint = toStored(QPointF(letterEnd.x(), letterEnd.y()));
    outPoint.ry() /= 2;
    limits = QRectF(toStored(letterBegin),
                    toStored(letterEnd));
    qreal width = limits.width();
    qreal height = limits.height();
    limits.setWidth(width / (1.0 + SvgView::scaleCanvasValueRef));
    limits.setHeight(height / (1.0 + SvgView::scaleCanvasValueRef));
    limits.moveLeft(toStored(letterBegin).x() + limits.width() * SvgView::scaleCanvasValueRef / 2);
    limits.moveTop(toStored(letterBegin).y() + limits.height() * SvgView::scaleCanvasValueRef / 2);
    limitsTopLeft = limits.topLeft();
    limitsBottomRight = limits.bottomRight();
    hideAll();
    showLetter = true;

    update();
}

void SvgEditor::scaleSVGCanvas(QString fileName)
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
    QSvgWidget::load(doc.toString(0).replace(">\n<tspan", "><tspan").toUtf8());
}

void SvgEditor::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->delta() / 240.0);
    qreal newFactor = scaleFactor * factor;

    if (newFactor < maxScaleFactor && newFactor > minScaleFactor)
        scaleFactor = newFactor;

    calculateCoordinates();
    update();
    event->accept();
}

void SvgEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    if (drawInPoint)
        setInPoint(event->pos());

    if (drawOutPoint)
        setOutPoint(event->pos());

    if (drawLimits)
        setLimitsTopLeft(event->pos());
}

void SvgEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (drawLimits)
        setLimitsBottomRight(event->pos());

    if (drawInPoint)
        setInPoint(event->pos());

    if (drawOutPoint)
        setOutPoint(event->pos());
}

void SvgEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (drawLimits)
    {
        setLimitsBottomRight(event->pos());

        if (limits.topLeft().x() > limits.bottomRight().x() ||
            limits.topLeft().y() > limits.bottomRight().y())
            limits = QRectF(limits.topRight(), limits.bottomLeft());
    }

    if (drawInPoint)
        setInPoint(event->pos());

    if (drawOutPoint)
        setOutPoint(event->pos());
}

void SvgEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawRect(0, 0, this->width() - 1, this->height() - 1);

    if (showLetter)
    {
        qreal realLetterWidth = renderer()->defaultSize().width() * scaleFactor;
        qreal realLetterHeight = renderer()->defaultSize().height() * scaleFactor;
        renderer()->render(&painter, QRectF(QPointF(width() / 2 - realLetterWidth / 2, height() / 2 - realLetterHeight / 2),
                                            renderer()->defaultSize() *= scaleFactor));
    }

    QRect rect(QPoint(), QSize(pointWidth, pointWidth));

    if (showInPoint)
    {
        painter.save();
        rect.moveCenter(fromStored(inPoint).toPoint());
        painter.setPen(Qt::cyan);
        painter.setBrush(Qt::darkCyan);
        painter.drawRect(rect);
        painter.restore();
    }

    if (showOutPoint)
    {
        painter.save();
        rect.moveCenter(fromStored(outPoint).toPoint());
        painter.setPen(Qt::magenta);
        painter.setBrush(Qt::darkMagenta);
        painter.drawRect(rect);
        painter.restore();
    }

    if (showLimits)
    {
        painter.save();
        painter.setPen(Qt::darkYellow);
        painter.drawRect(QRectF(fromStored(limitsTopLeft), fromStored(limitsBottomRight)));
        painter.restore();
    }

    painter.end();

    event->accept();
}

void SvgEditor::resizeEvent(QResizeEvent *event)
{
    QSvgWidget::resizeEvent(event);
    calculateCoordinates();
    update();
}

void SvgEditor::setLetterData(const QPointF _inPoint, const QPointF _outPoint, const QRectF _limits)
{
    if (_inPoint.x() >= 0 && _inPoint.y() >= 0)
        inPoint = _inPoint;

    if (_outPoint.x() >= 0 && _outPoint.y() >= 0)
        outPoint = _outPoint;

    if (_limits.left() >= 0 && _limits.top() >= 0 &&
        _limits.width() >= 0 && _limits.height() >= 0)
        limits = _limits;

    limitsTopLeft = limits.topLeft();
    limitsBottomRight = limits.bottomRight();
    showInPoint = true;
    showOutPoint = true;
    showLimits = true;
    update();
}

void SvgEditor::disableDrawing(const bool disable)
{
    if (disable)
    {
        drawInPoint = false;
        drawOutPoint = false;
        drawLimits = false;
        drawLetter = false;
    }
    update();
}

void SvgEditor::hideAll(const bool hide)
{
    if (hide)
    {
        showInPoint = false;
        showOutPoint = false;
        showLimits = false;
        showLetter = false;
    }
    update();
}

void SvgEditor::enableInPointDrawing(const bool draw)
{
    disableDrawing(true);
    drawInPoint = draw;
}

void SvgEditor::enableOutPointDrawing(const bool draw)
{
    disableDrawing(true);
    drawOutPoint = draw;
}

void SvgEditor::enableLimitsDrawing(const bool draw)
{
    disableDrawing(true);
    drawLimits = draw;
}

void SvgEditor::setInPoint(QPointF point)
{
    keepPointOnLetterCanvas(point);
    inPoint = toStored(point);
    showInPoint = true;
    update();
}

void SvgEditor::setOutPoint(QPointF point)
{
    keepPointOnLetterCanvas(point);
    outPoint = toStored(point);
    showOutPoint = true;
    update();
}

void SvgEditor::setLimitsTopLeft(QPointF point)
{
    keepPointOnLetterCanvas(point);
    limitsTopLeft = toStored(point);
}

void SvgEditor::setLimitsBottomRight(QPointF point)
{
    keepPointOnLetterCanvas(point);

    limitsBottomRight = toStored(point);

    limits = QRectF(limitsTopLeft, limitsBottomRight);
    showLimits = true;
    update();
}

QPointF SvgEditor::toStored(const QPointF &point)
{
    QPointF result;
    result.rx() = (point.x() - letterBegin.x()) / static_cast<qreal>(currentLetterSize.width() - 1);
    result.ry() = (point.y() - letterBegin.y()) / static_cast<qreal>(currentLetterSize.height() - 1);
    return result;
}

QPointF SvgEditor::fromStored(const QPointF &point)
{
    QPointF result;
    result.rx() = point.x() * static_cast<qreal>(currentLetterSize.width() - 1);
    result.ry() = point.y() * static_cast<qreal>(currentLetterSize.height() - 1);
    result += letterBegin;
    return result;
}

void SvgEditor::calculateCoordinates()
{
    letterBegin = QPointF(width() / 2.0 - renderer()->defaultSize().width() / 2.0 * scaleFactor,
                          height() / 2.0 - renderer()->defaultSize().height() / 2.0 * scaleFactor);
    currentLetterSize = renderer()->defaultSize() *= scaleFactor;
}

void SvgEditor::keepPointOnLetterCanvas(QPointF & point)
{
    if (point.x() < letterBegin.x())
        point.rx() = letterBegin.x();
    if (point.x() >= letterBegin.x() + currentLetterSize.width())
        point.rx() =  letterBegin.x() + currentLetterSize.width() - 1;
    if (point.y() < letterBegin.y())
        point.ry() = letterBegin.y();
    if (point.y() >= letterBegin.y() + currentLetterSize.height())
        point.ry() =  letterBegin.y() + currentLetterSize.height() - 1;
}
