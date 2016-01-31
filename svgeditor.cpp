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
    limitsTopLeft = limits.topLeft();
    limitsBottomRight = limits.bottomRight();
    drawInPoint = false;
    drawOutPoint = false;
    drawLimits = false;
    drawLetter = false;
}

void SvgEditor::load(const QString & file)
{
    QSvgWidget::load(file);
    letterSize = renderer()->defaultSize();
    //setFixedWidth(letterSize.width() * static_cast<qreal>(height()) / letterSize.height());
    drawLetter = true;
    inPoint = QPointF(-1.0, -1.0);
    outPoint = QPointF(-1.0, -1.0);
    limits = QRectF(-1.0,-1.0,-1.0,-1.0);
    limitsTopLeft = limits.topLeft();
    limitsBottomRight = limits.bottomRight();
    drawInPoint = false;
    drawOutPoint = false;
    drawLimits = false;
    showInPoint = false;
    showOutPoint = false;
    showLimits = false;
    update();
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
    qreal scale = 3;
    qreal wWidth = width()/scale, wHeight = height() / scale;
    QRect viewBox(-wWidth/2+letterSize.width()/2, -wHeight/2+letterSize.height()/2, wWidth, wHeight);
    renderer()->setViewBox(viewBox);
    if (drawLetter)
        QSvgWidget::paintEvent(event);

    QPainter painter(this);
    painter.drawRect(0, 0, this->width() - 1, this->height() - 1);
    painter.setViewport(viewBox);
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

void SvgEditor::disableDrawing(bool disable)
{
    if (disable)
    {
        drawInPoint = false;
        drawOutPoint = false;
        drawLimits = false;
    }
    update();
}

void SvgEditor::enableInPointDrawing(bool draw)
{
    disableDrawing(true);
    drawInPoint = draw;
}

void SvgEditor::enableOutPointDrawing(bool draw)
{
    disableDrawing(true);
    drawOutPoint = draw;
}

void SvgEditor::enableLimitsDrawing(bool draw)
{
    disableDrawing(true);
    drawLimits = draw;
}

void SvgEditor::setInPoint(const QPointF &point)
{
    inPoint = toStored(point);
    showInPoint = true;
    update();
}

void SvgEditor::setOutPoint(const QPointF &point)
{
    outPoint = toStored(point);
    showOutPoint = true;
    update();
}

void SvgEditor::setLimitsTopLeft(const QPointF &point)
{
    limitsTopLeft = toStored(point);
}

void SvgEditor::setLimitsBottomRight(const QPointF &point)
{
    limitsBottomRight = point;

    if (limitsBottomRight.x() > this->width())
        limitsBottomRight.rx() = this->width() - 1;

    if (limitsBottomRight.y() > this->height())
        limitsBottomRight.ry() = this->height() - 1;

    limitsBottomRight = toStored(limitsBottomRight);

    limits = QRectF(limitsTopLeft, limitsBottomRight);
    showLimits = true;
    update();
}

QPointF SvgEditor::toStored(const QPointF &point)
{
    QPointF result, poin;
    result.rx() = point.x() / static_cast<qreal>(this->width() - 1);
    result.ry() = point.y() / static_cast<qreal>(this->height() - 1);
    return result;
}

QPointF SvgEditor::fromStored(const QPointF &point)
{
    QPointF result, poin;
    result.rx() = point.x() * static_cast<qreal>(this->width() - 1);
    result.ry() = point.y() * static_cast<qreal>(this->height() - 1);
    return result;
}
