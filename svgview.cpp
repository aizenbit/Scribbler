#include "svgview.h"

SvgView::SvgView(QWidget *parent) : QGraphicsView(parent)
{
    currentScaleFactor = 1.0;
    maxScaleFactor = 1.5; //if this is exceeded, graphic artifacts will occure
    minScaleFactor = 0.05;
    changeMargins = false;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    setRenderHints(QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    limitScale(0.3);

    scene = new QGraphicsScene();
    setScene(scene);

    centerOn(0.0, 0.0);
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
    qreal newFactor = currentScaleFactor * factor;

    if (newFactor < maxScaleFactor && newFactor > minScaleFactor)
    {
        currentScaleFactor = newFactor;
        scale(factor, factor);
    }
}

int SvgView::renderText(const QStringRef &text)
{
    prepareSceneToRender();

    int endOfSheet = 0;

    //---Sequentially add the letters to the scene
    for (int currentSymbolNumber = 0; currentSymbolNumber < text.length(); currentSymbolNumber++)
    {
        QChar symbol = text.at(currentSymbolNumber);

        if (!font.contains(symbol))
        {
            processUnknownSymbol(symbol);
            endOfSheet++;
            continue;
        }

        letterData = font.values(symbol).at(qrand() % font.values(symbol).size());

        QGraphicsSvgItem *letterItem = new QGraphicsSvgItem(letterData.fileName);

        if (useCustomFontColor)
        {
            QGraphicsColorizeEffect *colorEffect = new QGraphicsColorizeEffect();
            colorEffect->setColor(fontColor);
            letterItem->setGraphicsEffect(colorEffect);
        }

        qreal letterHeight = letterItem->boundingRect().height() * letterData.limits.height();
        qreal letterWidth = letterItem->boundingRect().width() * letterData.limits.width();
        letterItem->setScale(fontSize * dpmm / letterHeight);
        letterWidth *= letterItem->scale();
        letterHeight *= letterItem->scale();

        letterBoundingSize.setWidth(letterItem->boundingRect().width() * letterItem->scale());
        letterBoundingSize.setHeight(letterItem->boundingRect().height() * letterItem->scale());

        cursor.rx() -= letterBoundingSize.width() * letterData.limits.topLeft().x();
        preventGoingBeyondRightMargin();

        //rendering stops by the end of sheet
        if (cursor.y() > currentMarginsRect.bottomRight().y() - fontSize * dpmm)
        {
            delete letterItem;
            return endOfSheet;
        }

        QPointF letterItemPos = cursor;
        letterItemPos.ry() -= letterBoundingSize.height() * letterData.limits.topLeft().y();
        letterItem->setPos(letterItemPos);
        scene->addItem(letterItem);

        if (connectLetters && lastLetter != nullptr)
            connectLastLetterToCurrent();

        lastLetter = letterItem;
        previousLetterCursor = cursor;
        previousLetterData = letterData;
        cursor.rx() += letterWidth + letterSpacing * dpmm;
        endOfSheet++;
    }

    return endOfSheet;
}

void SvgView::prepareSceneToRender()
{
    scene->clear();
    lastLetter = nullptr;

    if (changeMargins)
        currentMarginsRect = QRectF(QPointF(sheetRect.topRight().x() - marginsRect.topRight().x(),
                                            marginsRect.topLeft().y()),
                                    QPointF(sheetRect.bottomRight().x() - marginsRect.bottomLeft().x(),
                                            marginsRect.bottomRight().y()));
    else
        currentMarginsRect = marginsRect;

    scene->addRect(sheetRect);
    scene->addRect(currentMarginsRect, QPen(Qt::darkGray));

    cursor = QPointF(currentMarginsRect.x(), currentMarginsRect.y());
}

void SvgView::preventGoingBeyondRightMargin()
{
    //letter width is not yet known, so let it be equal to the height
    qreal letterHeight = fontSize * dpmm, letterWidth = letterHeight;

    if (cursor.x() > (currentMarginsRect.x() + currentMarginsRect.width() - letterWidth))
    {
        lastLetter = nullptr;
        cursor += QPointF(currentMarginsRect.x() - cursor.x(), letterHeight + lineSpacing * dpmm);
        cursor.rx() -= letterBoundingSize.width() * letterData.limits.topLeft().x();
    }
}

void SvgView::connectLastLetterToCurrent()
{
    QSize lastLetterBoundingRect;
    lastLetterBoundingRect.setWidth(lastLetter->boundingRect().width() * lastLetter->scale());
    lastLetterBoundingRect.setHeight(lastLetter->boundingRect().height() * lastLetter->scale());

    QPointF inPoint, outPoint;
    outPoint.rx() = previousLetterCursor.x() +
            previousLetterData.outPoint.x() * lastLetterBoundingRect.width();
    outPoint.ry() = previousLetterCursor.y() +
            previousLetterData.outPoint.y() * lastLetterBoundingRect.height() -
            lastLetterBoundingRect.height() * previousLetterData.limits.topLeft().y();

    inPoint.rx() = cursor.x() +
            letterData.inPoint.x() * letterBoundingSize.width();
    inPoint.ry() = cursor.y() +
            letterData.inPoint.y() * letterBoundingSize.height() -
            letterBoundingSize.height() * letterData.limits.topLeft().y();

    QPen pen(fontColor);
    pen.setWidth(0.4 * dpmm);
    pen.setCapStyle(Qt::RoundCap);

    scene->addLine(outPoint.x(), outPoint.y(), inPoint.x(), inPoint.y(), pen);
}

void SvgView::processUnknownSymbol(const QChar &symbol)
{
    switch (symbol.toLatin1())
    {
    case '\t':
        cursor += QPointF(fontSize * dpmm * spacesInTab, 0.0);
        lastLetter = nullptr;
        break;

    case '\n':
        cursor += QPointF(currentMarginsRect.x() - cursor.x(), (fontSize + lineSpacing) * dpmm);
        lastLetter = nullptr;
        break;

    default:
        cursor += QPointF((fontSize + letterSpacing) * dpmm, 0.0);
        lastLetter = nullptr;
        break;
    }
}

QImage SvgView::saveRenderToImage()
{
    QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter);

    return image;
}

void SvgView::loadFont(QString fontpath)
{
    if (fontpath.isEmpty())
    {
        QSettings settings("Settings.ini", QSettings::IniFormat);
        settings.beginGroup("Settings");
        fontpath = settings.value("last-used-font", QString()).toString();
        settings.endGroup();
    }

    if (fontpath.isEmpty())
        return;

    QSettings fontSettings(fontpath, QSettings::IniFormat);
    fontSettings.beginGroup("Font");
    fontSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    if (fontSettings.allKeys().size() == 0)
    {
        fontSettings.endGroup();
        return;
    }

    QString fontDirectory = QFileInfo(fontpath).path() + '/';
    font.clear();

    for (const QString &key : fontSettings.childKeys())
        for (Letter value : fontSettings.value(key).value<QList<Letter>>())
        {
            value.fileName = fontDirectory + value.fileName;
            font.insert(key.at(0).toLower(), value);
        }

    //It's a dirty hack, which helps to distinguish uppercase and lowercase
    //letters on freaking case-insensetive Windows
    fontSettings.beginGroup("UpperCase");
    for (const QString &key : fontSettings.childKeys())
        for (Letter value : fontSettings.value(key).value<QList<Letter>>())
        {
            value.fileName = fontDirectory + value.fileName;
            font.insert(key.at(0).toUpper(), value);
        }

    fontSettings.endGroup();

    fontSettings.endGroup();

    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    settings.setValue("last-used-font", QVariant(fontpath));
    settings.endGroup();
}

void SvgView::loadSettingsFromFile()
{
    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    dpi = settings.value("dpi").toInt();
    dpmm = dpi / 25.4;
    letterSpacing = settings.value("letter-spacing").toDouble();
    lineSpacing = settings.value("line-spacing").toDouble();
    spacesInTab = settings.value("spaces-in-tab").toInt();
    fontSize = settings.value("font-size").toDouble();
    sheetRect = QRectF(0, 0,
                       settings.value("sheet-width").toInt() * dpmm,
                       settings.value("sheet-height").toInt() * dpmm);

    marginsRect = QRectF(sheetRect.topLeft() + QPointF(settings.value("left-margin").toInt() * dpmm,
                                                       settings.value("top-margin").toInt() * dpmm),
                         sheetRect.bottomRight() - QPointF(settings.value("right-margin").toInt() * dpmm,
                                                           settings.value("bottom-margin").toInt() * dpmm));

    loadFont(settings.value("last-used-font", "Font/DefaultFont.ini").toString());

    fontColor = QColor(settings.value("font-color").toString());
    useCustomFontColor = settings.value("use-custom-font-color").toBool();
    connectLetters = settings.value("connect-letters").toBool();

    settings.endGroup();

    scene->setSceneRect(sheetRect);
    renderText();
}

void SvgView::hideBorders(bool hide)
{
    scene->items(Qt::AscendingOrder).at(0)->setVisible(!hide); //sheetRect
    scene->items(Qt::AscendingOrder).at(1)->setVisible(!hide); //marginsRect
}

void SvgView::changeLeftRightMargins(bool change)
{
    changeMargins = change;
}
