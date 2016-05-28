#include "svgview.h"

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
    loadHyphenRules();

    int endOfSheet = 0;

    //---Sequentially add the symbols to the scene
    for (int currentSymbolNumber = 0; currentSymbolNumber < text.length(); currentSymbolNumber++)
    {
        QChar symbol = text.at(currentSymbolNumber);

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

        symbolItem->setScale(data.scale);
        symbolBoundingSize = symbolItem->boundingRect().size() * symbolItem->scale();

        qreal symbolWidth = data.width * data.symbolData.limits.width() * data.scale;

        cursor.rx() -= symbolBoundingSize.width() * symbolData.limits.topLeft().x();
        preventGoingBeyondRightMargin(symbolWidth, text, currentSymbolNumber);

        //rendering stops by the end of sheet
        if (cursor.y() > currentMarginsRect.bottomRight().y() - fontSize * dpmm)
        {
            delete symbolItem;
            return endOfSheet;
        }

        QPointF symbolItemPos = cursor;

        symbolItemPos.ry() -= symbolBoundingSize.height() * symbolData.limits.top();
        symbolItem->setPos(symbolItemPos);
        scene->addItem(symbolItem);

        if (connectLetters && lastLetter != nullptr && symbol.isLetter())
            connectLastLetterToCurrent();

        lastLetter = symbolItem;
        previousSymbolCursor = cursor;
        previousSymbolData = symbolData;
        previousSymbolWidth = symbolWidth;
        cursor.rx() += symbolWidth + letterSpacing * dpmm;
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

void SvgView::preventGoingBeyondRightMargin(qreal symbolWidth, QStringRef text, int currentSymbolIndex)
{
    qreal symbolHeight = fontSize * dpmm;

    if (cursor.x() > (currentMarginsRect.x() + currentMarginsRect.width() - symbolWidth))
    {
        if (!hyphenate(text, currentSymbolIndex))
            wrapWords(text, currentSymbolIndex);

        if (!wordWrap && !hyphenateWords)
        {
            lastLetter = nullptr;
            cursor.rx() = currentMarginsRect.x() - symbolBoundingSize.width() * symbolData.limits.topLeft().x(); //WTF???
            cursor.ry() += symbolHeight + lineSpacing * dpmm;
        }
    }
}

void SvgView::wrapWords(QStringRef text, int currentSymbolIndex)
{
    qreal symbolHeight = fontSize * dpmm;
    int previousSymbolIndex = currentSymbolIndex - 1;

    if (!wordWrap || previousSymbolIndex < 0 ||
            previousSymbolIndex >= text.size() ||
            !(text.at(currentSymbolIndex).isLetterOrNumber() || text.at(currentSymbolIndex).isPunct()) ||
            !(text.at(previousSymbolIndex).isLetterOrNumber() || text.at(previousSymbolIndex).isPunct()))
        return;

    int lastSpace = text.toString().lastIndexOf(QRegularExpression("\\s"), currentSymbolIndex);
    int symbolsToWrap = currentSymbolIndex - lastSpace - 1;

    wrapLastSymbols(symbolsToWrap);

    cursor.rx() = previousSymbolCursor.x() + previousSymbolWidth;
    cursor.ry() += symbolHeight + lineSpacing * dpmm;
}

bool SvgView::hyphenate(QStringRef text, int currentSymbolIndex)
{
    qreal symbolHeight = fontSize * dpmm;
    int previousSymbolIndex = currentSymbolIndex - 1;

    if (!hyphenateWords || previousSymbolIndex < 0 ||
            previousSymbolIndex >= text.size() ||
            !text.at(currentSymbolIndex).isLetterOrNumber() ||
            !text.at(previousSymbolIndex).isLetterOrNumber() ||
            cursor.y() - previousSymbolCursor.y() > 0.0000001)
        return false;

    int lastSpace = text.toString().lastIndexOf(QRegularExpression("\\s"), currentSymbolIndex);
    int nextSpace = text.toString().indexOf(QRegularExpression("\\s"), currentSymbolIndex);
    QString word = text.mid(lastSpace + 1, nextSpace - lastSpace).toString();
    QString hypher = "\\1-\\2";
    QString hyphenWord = word;

    for (QRegularExpression &rule : hyphenRules)
        hyphenWord.replace(rule, hypher);

    int currentSymbolInWord = currentSymbolIndex - lastSpace - 1;

    for (int i = 0; i <= currentSymbolInWord; i++)
        if (hyphenWord.at(i) == '-')
            currentSymbolInWord++;

    qreal indexOfLastHyphen = hyphenWord.lastIndexOf('-', currentSymbolInWord);
    qreal symbolsToWrap = currentSymbolInWord - indexOfLastHyphen - 1;

    if (indexOfLastHyphen > 0 && symbolsToWrap > 0)
    {
        int itemsCount = scene->items().size();
        int itemsToWrap = symbolsToWrap;
        if (connectLetters)
            itemsToWrap = symbolsToWrap * 2 - 1;
        if (connectLetters && hyphenWord.at(currentSymbolInWord).isLetter())
            scene->removeItem(scene->items(Qt::AscendingOrder).at(itemsCount - itemsToWrap));

        QGraphicsSvgItem *hyphen = generateHyphen(symbolsToWrap);
        wrapLastSymbols(symbolsToWrap);

        if (hyphen != nullptr)
            scene->addItem(hyphen);
    }
    else
        return false;

    cursor.rx() = previousSymbolCursor.x() + previousSymbolWidth;
    cursor.ry() += symbolHeight + lineSpacing * dpmm;

    return true;
}

void SvgView::wrapLastSymbols(int symbolsToWrap)
{
    if (symbolsToWrap <= 0 || symbolsToWrap >= scene->items().size())
        return;

    int itemsCount = scene->items().size();
    int itemsToWrap = symbolsToWrap;
    qreal symbolHeight = fontSize * dpmm;

    if (connectLetters)
        itemsToWrap = symbolsToWrap * 2 - 1;

    qreal leftOffset = scene->items(Qt::AscendingOrder)[itemsCount - itemsToWrap]->pos().x() - currentMarginsRect.x();

    if (connectLetters)
    {
        previousSymbolCursor.rx() -= leftOffset;
        previousSymbolCursor.ry() += symbolHeight + lineSpacing * dpmm;
    }

    for (int i = itemsToWrap; i > 0; i--)
    {
        QPointF pos = scene->items(Qt::AscendingOrder)[itemsCount - i]->pos();
        pos.rx() -= leftOffset;
        pos.ry() += symbolHeight + lineSpacing * dpmm;
        scene->items(Qt::AscendingOrder)[itemsCount - i]->setPos(pos);
    }
}

QGraphicsSvgItem * SvgView::generateHyphen(int symbolsToWrap)
{
    if (!font.contains('-'))
        return nullptr;

    if (connectLetters)
        symbolsToWrap = symbolsToWrap * 2 - 1;

    QGraphicsSvgItem *hyphen = new QGraphicsSvgItem();

    SvgData data = font.values('-').at(qrand() % font.values('-').size());
    hyphen->setSharedRenderer(data.renderer);
    hyphen->setScale(data.scale);

    SymbolData hyphenData = data.symbolData;

    QSizeF  hyphenBoundingSize = hyphen->boundingRect().size() * hyphen->scale();
    QGraphicsItem* nearestLetter = scene->items(Qt::AscendingOrder).at(scene->items().size() - symbolsToWrap);
    QPointF hyphenPos = cursor;
    hyphenPos.ry() -= hyphenBoundingSize.height() * hyphenData.limits.top();
    hyphenPos.rx() = nearestLetter->pos().x() + nearestLetter->boundingRect().width();
    hyphen->setPos(hyphenPos);

    return hyphen;
}

void SvgView::connectLastLetterToCurrent()
{
    QSize lastLetterBoundingRect;
    lastLetterBoundingRect.setWidth(lastLetter->boundingRect().width() * lastLetter->scale());
    lastLetterBoundingRect.setHeight(lastLetter->boundingRect().height() * lastLetter->scale());

    QPointF inPoint, outPoint;
    outPoint.rx() = previousSymbolCursor.x() +
            previousSymbolData.outPoint.x() * lastLetterBoundingRect.width();
    outPoint.ry() = previousSymbolCursor.y() +
            previousSymbolData.outPoint.y() * lastLetterBoundingRect.height() -
            lastLetterBoundingRect.height() * previousSymbolData.limits.topLeft().y();

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
        cursor.rx() += fontSize * dpmm * spacesInTab;
        lastLetter = nullptr;
        break;

    case '\n':
        cursor.rx() = currentMarginsRect.x();
        cursor.ry() += (fontSize + lineSpacing) * dpmm;
        lastLetter = nullptr;
        break;

    default:
        cursor.rx() += (fontSize + letterSpacing) * dpmm;
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
        for (SymbolData symbolData : fontSettings.value(key).value<QList<SymbolData>>())
        {
            symbolData.fileName = fontDirectory + symbolData.fileName;
            if (key == "slash")
                insertSymbol('/', symbolData);
            else if (key == "backslash")
                insertSymbol('\\', symbolData);
            else
                insertSymbol(key.at(0), symbolData);
        }

    //It's a dirty hack, which helps to distinguish uppercase and lowercase
    //letters on a freaking case-insensetive Windows
    fontSettings.beginGroup("UpperCase");
    for (const QString &key : fontSettings.childKeys())
        for (SymbolData symbolData : fontSettings.value(key).value<QList<SymbolData>>())
        {
            symbolData.fileName = fontDirectory + symbolData.fileName;
            insertSymbol(key.at(0), symbolData);
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
    qreal symbolHeight = renderer->defaultSize().height() * symbolData.limits.height();
    qreal scale = fontSize * dpmm / symbolHeight;

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
    qreal width = renderer->defaultSize().width() + renderer->defaultSize().width() / 2;
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
    QStringList viewBoxValues = svgElement.attribute("viewBox").split(" ");

    if (viewBoxValues.isEmpty())
        return;

    qreal width = viewBoxValues.at(2).toDouble() - viewBoxValues.at(0).toDouble();
    qreal height = viewBoxValues.at(3).toDouble() - viewBoxValues.at(1).toDouble();

    QString viewBox = QString("%1 %2 %3 %4")
            .arg(static_cast<qreal>(viewBoxValues.at(0).toDouble() - width / 2))
            .arg(static_cast<qreal>(viewBoxValues.at(1).toDouble() - height / 2))
            .arg(static_cast<qreal>(viewBoxValues.at(2).toDouble() + width))
            .arg(static_cast<qreal>(viewBoxValues.at(3).toDouble() + height));

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
    useSeed =       settings.value("use-seed").toBool();
    seed =          settings.value("seed").toInt();
    wordWrap =      settings.value("wrap-words").toBool();
    useCustomFontColor = settings.value("use-custom-font-color").toBool();
    connectLetters =     settings.value("connect-letters").toBool();
    hyphenateWords =     settings.value("hyphenate-words").toBool();

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

void SvgView::loadHyphenRules()
{
    hyphenRules.clear();
    QMap<QString, QString> variables;
    QSettings settings("hyphenationRules.ini", QSettings::IniFormat);
    settings.beginGroup("Variables");

    for (const QString &name : settings.childKeys())
        variables.insert(name, QString::fromUtf8(settings.value(name).toString().toLatin1()));

    settings.endGroup();
    settings.beginGroup("Rules");
    for (const QString &key : settings.childKeys())
    {
        QString rule = settings.value(key).toString();

        for (QString &variable : variables.uniqueKeys())
            rule.replace(variable, variables[variable]);

        hyphenRules.push_back(QRegularExpression(rule));
    }

    settings.endGroup();
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
