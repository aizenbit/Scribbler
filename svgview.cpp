#include "svgview.h"

const qreal &SvgView::scaleCanvasValueRef = SvgView::scaleCanvasValue;
qreal SvgView::scaleCanvasValue;
bool SvgView::scaleCanvas;

SvgView::SvgView(QWidget *parent) : QGraphicsView(parent)
{
    currentScaleFactor = 1.0;
    maxScaleFactor = 1.5; //if this is exceeded, graphic artifacts will occure
    minScaleFactor = 0.05;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    setRenderHints(QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    limitScale(0.3);

    scene = new QGraphicsScene();
    setScene(scene);

    centerOn(0.0, 0.0);

    specialSymbols << '.' << ',';
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

    //---Sequentially add the symbols to the scene
    for (int currentSymbolNumber = 0; currentSymbolNumber < text.length(); currentSymbolNumber++)
    {
        QChar symbol = text.at(currentSymbolNumber);
        bool isSpecialSymbol = specialSymbols.contains(symbol);

        if (!font.contains(symbol))
        {
            processUnknownSymbol(symbol);
            endOfSheet++;

            if (cursor.x() > currentMarginsRect.bottomRight().x() - (fontSize + letterSpacing) * dpmm)
                cursor += QPointF(currentMarginsRect.x() - cursor.x(), (fontSize + lineSpacing) * dpmm);
            if (cursor.y() > currentMarginsRect.bottomRight().y() - fontSize * dpmm)
                return endOfSheet;

            continue;
        }

        QGraphicsSvgItem *symbolItem = new QGraphicsSvgItem();
        SvgData data = font.values(symbol).at(qrand() % font.values(symbol).size());
        symbolItem->setSharedRenderer(data.renderer);
        symbolData = data.symbolData;

        if (isSpecialSymbol)
            data.scale /= 5;

        symbolItem->setScale(data.scale);
        symbolBoundingSize = symbolItem->boundingRect().size() * symbolItem->scale();

        cursor.rx() -= symbolBoundingSize.width() * symbolData.limits.topLeft().x();
        qreal letterWidth = data.width * data.symbolData.limits.width() * data.scale;
        preventGoingBeyondRightMargin(letterWidth);

        //rendering stops by the end of sheet
        if (cursor.y() > currentMarginsRect.bottomRight().y() - fontSize * dpmm)
        {
            delete symbolItem;
            return endOfSheet;
        }

        QPointF symbolItemPos = cursor;

        if (isSpecialSymbol)
            symbolItemPos.ry() += fontSize * dpmm - symbolBoundingSize.height() * symbolData.limits.bottomRight().y();
        else
            symbolItemPos.ry() -= symbolBoundingSize.height() * symbolData.limits.topLeft().y();

        symbolItem->setPos(symbolItemPos);
        scene->addItem(symbolItem);

        if (connectLetters && lastLetter != nullptr && symbol.isLetter())
            connectLastLetterToCurrent();

        lastLetter = symbolItem;
        previousLetterCursor = cursor;
        previousLetterData = symbolData;
        cursor.rx() += letterWidth + letterSpacing * dpmm;
        endOfSheet++;

        if (!symbol.isLetter())
            lastLetter = nullptr;
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

    if (useSeed)
        srand(seed);
    else
        srand(QTime::currentTime().msec());
}

void SvgView::preventGoingBeyondRightMargin(qreal letterWidth)
{
    qreal letterHeight = fontSize * dpmm;

    if (cursor.x() > (currentMarginsRect.x() + currentMarginsRect.width() - letterWidth))
    {
        lastLetter = nullptr;
        cursor += QPointF(currentMarginsRect.x() - cursor.x(), letterHeight + lineSpacing * dpmm);
        cursor.rx() -= symbolBoundingSize.width() * symbolData.limits.topLeft().x();
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
            symbolData.inPoint.x() * symbolBoundingSize.width();
    inPoint.ry() = cursor.y() +
            symbolData.inPoint.y() * symbolBoundingSize.height() -
            symbolBoundingSize.height() * symbolData.limits.topLeft().y();

    QPen pen(fontColor);
    pen.setWidth(penWidth * dpmm);
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

    for (SvgData &data : font.values())
    {
        delete data.renderer;
        data.renderer = nullptr;
    }
    font.clear();

    QString fontDirectory = QFileInfo(fontpath).path() + '/';

    for (const QString &key : fontSettings.childKeys())
        for (SymbolData letterData : fontSettings.value(key).value<QList<SymbolData>>())
        {
            letterData.fileName = fontDirectory + letterData.fileName;
            if (key == "slash")
                insertSymbol('/', letterData);
            else if (key == "backslash")
                insertSymbol('\\', letterData);
            else
                insertSymbol(key.at(0), letterData);
        }

    //It's a dirty hack, which helps to distinguish uppercase and lowercase
    //letters on a freaking case-insensetive Windows
    fontSettings.beginGroup("UpperCase");
    for (const QString &key : fontSettings.childKeys())
        for (SymbolData letterData : fontSettings.value(key).value<QList<SymbolData>>())
        {
            letterData.fileName = fontDirectory + letterData.fileName;
            insertSymbol(key.at(0), letterData);
        }

    fontSettings.endGroup();
    fontSettings.endGroup();

    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    settings.setValue("last-used-font", QVariant(fontpath));
    settings.endGroup();
}

void SvgView::insertSymbol(QChar key, SymbolData &symbolData)
{
    QSvgRenderer *renderer = new QSvgRenderer(symbolData.fileName);
    qreal letterHeight = renderer->defaultSize().height() * symbolData.limits.height();
    qreal scale = fontSize * dpmm / letterHeight;

    QDomDocument doc("SVG");
    QFile file(symbolData.fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }

    file.close();

    QDomElement svgElement = doc.elementsByTagName("svg").item(0).toElement();
    scaleViewBox(svgElement);

    QStringList viewBox = svgElement.attribute("viewBox").split(" ");
    qreal dotsPerUnits = renderer->defaultSize().height() / viewBox.at(3).toDouble();
    qreal newPenWidth = penWidth * dpmm / scale / dotsPerUnits;

    if (specialSymbols.contains(key))
        newPenWidth *= 5; //yeah, magic number

    QDomNodeList pathList = doc.elementsByTagName("path");
    QDomNodeList styleList = doc.elementsByTagName("style");
    QDomNodeList gList = doc.elementsByTagName("g");

    if (!styleList.isEmpty())
    {
        QDomElement element = styleList.item(0).toElement();
        QString style = element.text();
        changeAttribute(style, "stroke-width", QString("%1").arg(newPenWidth));
        if (useCustomFontColor)
            changeAttribute(style, "stroke", fontColor.name(QColor::HexRgb));
        if (roundLines)
        {
            changeAttribute(style, "stroke-linecap", "round");
            changeAttribute(style, "stroke-linejoin", "round");
        }
        QDomElement newElement = doc.createElement("style");
        QDomCDATASection newText = doc.createCDATASection(style);
        newElement.appendChild(newText);
        newElement.setAttribute("type", element.attribute("type", ""));
        element.parentNode().replaceChild(newElement, element);
    }
    else
        for (int i = 0; i < pathList.count(); i++)
        {
            QDomElement element = pathList.at(i).toElement();
            QString style = element.attribute("style", "");
            changeAttribute(style, "stroke-width", QString("%1").arg(newPenWidth));
            if (useCustomFontColor)
                changeAttribute(style, "stroke", fontColor.name(QColor::HexRgb));
            if (roundLines)
            {
                changeAttribute(style, "stroke-linecap", "round");
                changeAttribute(style, "stroke-linejoin", "round");
            }
            element.setAttribute("style", style);
        }

    renderer->load(doc.toString(0).replace(">\n<tspan", "><tspan").toUtf8());
    qreal width = renderer->defaultSize().width() + renderer->defaultSize().width() * scaleCanvasValue / 2;
    font.insert(key, {symbolData, scale, width, renderer});
}

void SvgView::changeAttribute(QString &attribute, QString parameter, QString newValue)
{
    if (attribute.contains(QRegularExpression(parameter + ":")))
    {
        int index = attribute.indexOf(parameter + ":");
        int endSign = attribute.indexOf(QRegularExpression(";|}"), index);
        int valueBegin = index + parameter.size() + 1;

        attribute.remove(valueBegin, endSign - valueBegin);
        attribute.insert(valueBegin, newValue);
    }
    else
    {
        int semicolon = attribute.lastIndexOf(QRegularExpression(";"));
        int endSign = attribute.lastIndexOf(QRegularExpression(";|}"));
        attribute.insert(semicolon > endSign ? semicolon : endSign, (attribute.isEmpty() ? "" : ";") + parameter + ":" + newValue);
    }
}

void SvgView::scaleViewBox(QDomElement &svgElement)
{
    if (!scaleCanvas)
        return;

    QStringList viewBoxValues = svgElement.attribute("viewBox").split(" ");

    if (viewBoxValues.isEmpty())
        return;

    qreal width = viewBoxValues.at(2).toDouble() - viewBoxValues.at(0).toDouble();
    qreal height = viewBoxValues.at(3).toDouble() - viewBoxValues.at(1).toDouble();

    QString viewBox = QString("%1 %2 %3 %4")
            .arg(static_cast<qreal>(viewBoxValues.at(0).toDouble() - width * scaleCanvasValue / 2))
            .arg(static_cast<qreal>(viewBoxValues.at(1).toDouble() - height * scaleCanvasValue / 2))
            .arg(static_cast<qreal>(viewBoxValues.at(2).toDouble() + width * scaleCanvasValue))
            .arg(static_cast<qreal>(viewBoxValues.at(3).toDouble() + height * scaleCanvasValue));

    svgElement.setAttribute("viewBox", viewBox);
}

void SvgView::loadSettingsFromFile()
{
    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");
    dpi = settings.value("dpi").toInt();
    dpmm = dpi / 25.4;
    letterSpacing = settings.value("letter-spacing").toDouble();
    lineSpacing =   settings.value("line-spacing").toDouble();
    spacesInTab =   settings.value("spaces-in-tab").toInt();
    fontSize =      settings.value("font-size").toDouble();
    penWidth =      settings.value("pen-width").toDouble();
    roundLines =    settings.value("round-lines").toBool();
    scaleCanvas =   settings.value("scale-canvas").toBool();
    useSeed =       settings.value("use-seed").toBool();
    seed =          settings.value("seed").toInt();
    scaleCanvasValue =   settings.value("scale-canvas-value").toDouble();
    useCustomFontColor = settings.value("use-custom-font-color").toBool();
    connectLetters =     settings.value("connect-letters").toBool();

    sheetRect = QRectF(0, 0,
                       settings.value("sheet-width").toInt() * dpmm,
                       settings.value("sheet-height").toInt() * dpmm);

    marginsRect = QRectF(sheetRect.topLeft() + QPointF(settings.value("left-margin").toInt() * dpmm,
                                                       settings.value("top-margin").toInt() * dpmm),
                         sheetRect.bottomRight() - QPointF(settings.value("right-margin").toInt() * dpmm,
                                                           settings.value("bottom-margin").toInt() * dpmm));

    fontColor = QColor(settings.value("font-color").toString());

    loadFont(settings.value("last-used-font", "Font/DefaultFont.ini").toString());
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
