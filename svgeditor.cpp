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
    minScaleFactor = 0.5;
    maxScaleFactor = 20;
    limitsTopLeft = limits.topLeft();
    limitsBottomRight = limits.bottomRight();
    disableDrawing();
    hideAll();
}

void SvgEditor::load(const QString & file)
{
    QSvgWidget::load(file);
    drawLetter = true;
    inPoint = QPointF(-1.0, -1.0);
    outPoint = QPointF(-1.0, -1.0);
    limits = QRectF(-1.0,-1.0,-1.0,-1.0);
    limitsTopLeft = limits.topLeft();
    limitsBottomRight = limits.bottomRight();
    disableDrawing();
    hideAll();
    showLetter = true;
    calculateCoordinates();
    update();
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

    QPen pen = QPen(Qt::SolidPattern, pointWidth);

    if (showInPoint)
    {
        painter.save();
        pen.setColor(Qt::darkCyan);
        painter.setPen(pen);
        painter.drawPoint(fromStored(inPoint));
        painter.restore();
    }

    if (showOutPoint)
    {
        painter.save();
        pen.setColor(Qt::darkMagenta);
        painter.setPen(pen);
        painter.drawPoint(fromStored(outPoint));
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
    inPoint = _inPoint;
    outPoint = _outPoint;
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
