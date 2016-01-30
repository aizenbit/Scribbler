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
    drawInPoint = false;
    drawOutPoint = false;
    drawLimits = false;
    drawLetter = false;
}

void SvgEditor::load(const QString & file)
{
    QSvgWidget::load(file);
    QSize letterSize = renderer()->defaultSize();
    setFixedWidth(letterSize.width() * static_cast<qreal>(height()) / letterSize.height());
    drawLetter = true;
    inPoint = QPointF(-1.0, -1.0);
    outPoint = QPointF(-1.0, -1.0);
    limits = QRectF(-1.0,-1.0,-1.0,-1.0);
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
    if (drawLetter)
        QSvgWidget::paintEvent(event);

    QPainter painter(this);
    painter.drawRect(0, 0, this->width() - 1, this->height() - 1);

    if (showInPoint)
    {
        painter.save();
        painter.setPen(Qt::darkCyan);
        painter.setBrush(Qt::darkCyan);
        painter.drawRect(inPoint.x() - pointWidth / 2, inPoint.y() - pointWidth / 2, 5, 5);
        painter.restore();
    }

    if (showOutPoint)
    {
        painter.save();
        painter.setPen(Qt::darkMagenta);
        painter.setBrush(Qt::darkMagenta);
        painter.drawRect(outPoint.x() - pointWidth / 2, outPoint.y() - pointWidth / 2, 5, 5);
        painter.restore();
    }


    if(showLimits)
    {
        painter.save();
        painter.setPen(Qt::darkYellow);
        painter.drawRect(limits);
        painter.restore();
    }

    painter.end();
}

void SvgEditor::setLetterData(const QPointF _inPoint, const QPointF _outPoint, const QRectF _limits)
{
    inPoint = _inPoint;
    outPoint = _outPoint;
    limits = _limits;
    showInPoint = true;
    showOutPoint = true;
    showLimits = true;
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
    inPoint = point;
    showInPoint = true;
    update();
}

void SvgEditor::setOutPoint(const QPointF &point)
{
    outPoint = point;
    showOutPoint = true;
    update();
}

void SvgEditor::setLimitsTopLeft(const QPointF &point)
{
    limitsTopLeft = point;
}

void SvgEditor::setLimitsBottomRight(const QPointF &point)
{
    limitsBottomRight = point;

    if (limitsBottomRight.x() > this->width())
        limitsBottomRight.rx() = this->width() - 1;

    if (limitsBottomRight.y() > this->height())
        limitsBottomRight.ry() = this->height() - 1;

    limits = QRectF(limitsTopLeft, limitsBottomRight);
    showLimits = true;
    update();
}
