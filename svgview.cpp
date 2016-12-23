#include "svgview.h"

SvgView::SvgView(QWidget *parent) : QGraphicsView(parent)
{
    currentScaleFactor = 1.0;
    areBordersHidden = false;
    maxScaleFactor = 1.5;
    minScaleFactor = 0.05;
    itemsToRemove = 0;

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
    drawMarking();
    int endOfSheet = 0;

    //Sequentially add the symbols to the scene
    for (int currentSymbolNumber = 0; currentSymbolNumber < text.length(); currentSymbolNumber++)
    {
        QChar symbol = text.at(currentSymbolNumber);
        randomizeLetterSpacing();

        if (!font.contains(symbol))
        {
            processUnknownSymbol(symbol);
            endOfSheet++;

            if (cursor.x() > currentMarginsRect.bottomRight().x() - (fontSize + currentLetterSpacing) * dpmm)
            {
                cursorToNewLine();
                storedWordItems.push_back(QVector<QGraphicsSvgItem *>());
                storedSymbolData.push_back(QVector<SymbolData>());
            }

            if (cursor.y() > currentMarginsRect.bottomRight().y() - fontSize * dpmm)
                break;

            continue;
        }

        QGraphicsSvgItem *symbolItem = new QGraphicsSvgItem();

        SvgData data = font.values(symbol).at(qrand() % font.values(symbol).size());
        symbolData = data.symbolData;

        symbolItem->setSharedRenderer(data.renderer);
        symbolItem->setScale(data.scale);

        symbolBoundingSize = symbolItem->boundingRect().size() * symbolItem->scale();
        qreal symbolWidth = symbolBoundingSize.width() * symbolData.limits.width();

        preventGoingBeyondRightMargin(symbolWidth, text, currentSymbolNumber);

        //rendering stops by the end of sheet
        if (cursor.y() > currentMarginsRect.bottomRight().y() - fontSize * dpmm)
        {
            delete symbolItem;
            break;
        }

        QPointF symbolItemPos = cursor;
        symbolItemPos.rx() -= symbolBoundingSize.width() * symbolData.limits.left();
        symbolItemPos.ry() -= symbolBoundingSize.height() * symbolData.limits.top();
        symbolItemPos += symbolPositionRandomValue();
        symbolItem->setPos(symbolItemPos);
        scene->addItem(symbolItem);

        previousSymbolCursor = cursor;
        previousSymbolData = symbolData;
        previousSymbolWidth = symbolWidth;
        cursor.rx() += symbolWidth + currentLetterSpacing * dpmm;
        endOfSheet++;

        if (symbol.isLetter())
        {
            storedSymbolData.last().push_back(symbolData);
            storedWordItems.last().push_back(symbolItem);
        }
        else
        {
            storedWordItems.push_back(QVector<QGraphicsSvgItem *>());
            storedSymbolData.push_back(QVector<SymbolData>());
        }

    }

    removeLastSymbols();
    endOfSheet -= itemsToRemove;
    connectLetters();

    return endOfSheet;
}

void SvgView::removeLastSymbols()
{
    uint itemsCount = scene->items().size();

    if (itemsCount < itemsToRemove)
        return;

    for (uint i = 0; i < itemsToRemove; i++)
    {
        while (storedSymbolData.last().isEmpty())
            storedSymbolData.removeLast();
        while (storedWordItems.last().isEmpty())
            storedWordItems.removeLast();

        scene->removeItem(storedWordItems.last().last());
        storedSymbolData.last().removeLast();
        storedWordItems.last().removeLast();
    }
}

void SvgView::prepareSceneToRender()
{
    scene->clear();
    storedSymbolData.clear();
    storedWordItems.clear();
    storedWordItems.push_back(QVector<QGraphicsSvgItem *>());
    storedSymbolData.push_back(QVector<SymbolData>());
    itemsToRemove = 0;

    currentMarginsRect = changedVerticalMargins();

    scene->addRect(sheetRect);
    scene->addRect(currentMarginsRect, QPen(Qt::darkGray));

    if (useSeed)
        qsrand(seed);
    else
        qsrand(QTime::currentTime().msec());

    hideBorders(areBordersHidden);

    randomizeMargins();
    cursor = QPointF(currentMarginsRect.x(), currentMarginsRect.y());
}

bool SvgView::preventGoingBeyondRightMargin(qreal symbolWidth, QStringRef text, int currentSymbolIndex)
{
    if (cursor.x() > (currentMarginsRect.x() + currentMarginsRect.width() - symbolWidth))
    {
        bool hyphenateHappened = hyphenate(text, currentSymbolIndex);
        bool wrapWordHappened = false;

        if (!hyphenateHappened)
            wrapWordHappened = wrapWords(text, currentSymbolIndex);

        if (!hyphenateHappened && !wrapWordHappened &&
                !text.at(currentSymbolIndex).isPunct())
        {
            cursorToNewLine();
            storedWordItems.push_back(QVector<QGraphicsSvgItem *>());
            storedSymbolData.push_back(QVector<SymbolData>());
        }

        return true;
    }

    return false;
}

bool SvgView::wrapWords(QStringRef text, int currentSymbolIndex)
{
    int previousSymbolIndex = currentSymbolIndex - 1;

    if (!wordWrap ||
            previousSymbolIndex < 0 || previousSymbolIndex >= text.size() ||
            !(text.at(currentSymbolIndex).isLetterOrNumber() || text.at(currentSymbolIndex).isPunct()) ||
            !(text.at(previousSymbolIndex).isLetterOrNumber() || text.at(previousSymbolIndex).isPunct()))
        return false;

    //wrap all the last letters
    int lastNonLetter = text.toString().lastIndexOf(QRegularExpression("[^\\p{L}]"), currentSymbolIndex);
    int symbolsToWrap = currentSymbolIndex - lastNonLetter - 1;

    if (wrapLastSymbols(symbolsToWrap))
    {
        cursor.rx() = previousSymbolCursor.x() + previousSymbolWidth;
        cursor.ry() += (fontSize + lineSpacing) * dpmm;

        if (cursor.y() > currentMarginsRect.bottomRight().y() - (lineSpacing + fontSize) * dpmm)
            itemsToRemove = symbolsToWrap;

        return true;
    }

    return false;
}

bool SvgView::hyphenate(QStringRef text, int currentSymbolIndex)
{
    int previousSymbolIndex = currentSymbolIndex - 1;

    if (!hyphenateWords || previousSymbolIndex < 0 ||
            previousSymbolIndex >= text.size() ||
            !text.at(currentSymbolIndex).isLetterOrNumber() ||
            !text.at(previousSymbolIndex).isLetterOrNumber() ||
            cursor.y() - previousSymbolCursor.y() > 0.0000001)
        return false;

    //find word boundary
    int lastNonLetter = text.toString().lastIndexOf(QRegularExpression("[^\\p{L}]"), currentSymbolIndex);
    int nextNonLetter = text.toString().indexOf(QRegularExpression("[^\\p{L}]"), currentSymbolIndex);
    QString word;

    //select a word
    if (nextNonLetter > 0)
        word = text.mid(lastNonLetter + 1, nextNonLetter - lastNonLetter).toString();
    else
        word = text.mid(lastNonLetter + 1, text.size() - 1).toString();

    QString hypher = "\\1-\\2";
    QString hyphenWord = word;

    //divide word into syllables with hyphens
    for (QRegularExpression &rule : hyphenRules)
        hyphenWord.replace(rule, hypher);

    int currentSymbolInWord = currentSymbolIndex - lastNonLetter - 1;

    //find new position of current letter
    for (int i = 0; i <= currentSymbolInWord && currentSymbolInWord < hyphenWord.size(); i++)
        if (hyphenWord.at(i) == '-')
            currentSymbolInWord++;

    //find hyphen, which is nearest to current letter
    qreal indexOfLastHyphen = hyphenWord.lastIndexOf('-', currentSymbolInWord);
    qreal symbolsToWrap = currentSymbolInWord - indexOfLastHyphen - 1;

    //generate hyphens item and hyphenate
    if (indexOfLastHyphen > 0 && symbolsToWrap >= 0)
    {
        QGraphicsSvgItem *hyphen = generateHyphen(symbolsToWrap);

        if (!wrapLastSymbols(symbolsToWrap))
        {
            randomizeMargins();
            previousSymbolCursor.rx() = currentMarginsRect.x() - previousSymbolWidth;
            previousSymbolCursor.ry() += (fontSize + lineSpacing) * dpmm;
            storedWordItems.push_back(QVector<QGraphicsSvgItem *>());
            storedSymbolData.push_back(QVector<SymbolData>());
        }

        if (hyphen != nullptr)
            scene->addItem(hyphen);
    }
    else
        return false;

    cursor.rx() = previousSymbolCursor.x() + previousSymbolWidth;
    cursor.ry() = previousSymbolCursor.y();

    if (cursor.y() > currentMarginsRect.bottomRight().y() - (lineSpacing + fontSize) * dpmm)
        itemsToRemove = symbolsToWrap;

    return true;
}

bool SvgView::wrapLastSymbols(int symbolsToWrap)
{
    if (symbolsToWrap <= 0 || symbolsToWrap >= scene->items().size())
        return false;

    //find the first item to wrap and it's position
    int itemsCount = scene->items().size();
    int itemsToWrap = symbolsToWrap; //TODO: consider missing items
    QGraphicsItem * firstItemToWrap = scene->items(Qt::AscendingOrder)[itemsCount - itemsToWrap];
    qreal firstWrapItemPosX = firstItemToWrap->pos().x();

    /*if (firstWrapItemPosX == currentMarginsRect.x())
        return false;*/

    randomizeMargins();
    //this is how much you need to move symbols to the left
    qreal leftOffset = firstWrapItemPosX - currentMarginsRect.x();

    if (storedSymbolData.last().size() - symbolsToWrap >= 0) //TODO: consider missing items and get rid of this check
    {
        SymbolData firstItemToWrapData = storedSymbolData.last().at(storedSymbolData.last().size() - symbolsToWrap);
        leftOffset += firstItemToWrap->boundingRect().size().width() * firstItemToWrap->scale() * firstItemToWrapData.limits.left();
    }

    previousSymbolCursor.rx() -= leftOffset;
    previousSymbolCursor.ry() += (fontSize + lineSpacing) * dpmm;
    storedWordItems.push_back(QVector<QGraphicsSvgItem *>());
    storedSymbolData.push_back(QVector<SymbolData>());

    //transfer symbols in a new column to half of
    //the wrapped word were not connected with the line
    for (int i = itemsToWrap; i > 0; i--)
    {
        int size = storedWordItems.size();
        int wordSize = storedWordItems.at(size - 2).size();

        if (wordSize <= i)
            break;

        storedWordItems.last().push_back(storedWordItems[size - 2].takeAt(wordSize - i));
        storedSymbolData.last().push_back(storedSymbolData[size - 2].takeAt(wordSize - i));
    }

    //wrap items
    for (int i = itemsToWrap; i > 0; i--)
    {
        QPointF pos = scene->items(Qt::AscendingOrder)[itemsCount - i]->pos();
        pos.rx() -= leftOffset;
        pos.ry() += (fontSize + lineSpacing) * dpmm;
        scene->items(Qt::AscendingOrder)[itemsCount - i]->setPos(pos);
    }

    return true;
}

QGraphicsSvgItem * SvgView::generateHyphen(int symbolsToWrap)
{
    if (!font.contains('-'))
        return nullptr;

    symbolsToWrap++;

    if (symbolsToWrap <= 0)
        return nullptr;

    QGraphicsSvgItem *hyphen = new QGraphicsSvgItem();

    //prepare hyphens item
    SvgData data = font.values('-').at(qrand() % font.values('-').size());
    hyphen->setSharedRenderer(data.renderer);
    hyphen->setScale(data.scale);

    SymbolData hyphenData = data.symbolData;

    //calculate hyphens position
    QSizeF  hyphenBoundingSize = hyphen->boundingRect().size() * hyphen->scale();
    QGraphicsItem* nearestLetter = scene->items(Qt::AscendingOrder).at(scene->items().size() - symbolsToWrap);
    QPointF hyphenPos = cursor;
    hyphenPos.ry() -= hyphenBoundingSize.height() * hyphenData.limits.top();
    hyphenPos.rx() = nearestLetter->pos().x() + nearestLetter->boundingRect().width() * nearestLetter->scale();

    if (storedSymbolData.last().size() - symbolsToWrap >= 0) //TODO: consider missing items and get rid of this check
    {
        SymbolData nearestLetterData = storedSymbolData.last().at(storedSymbolData.last().size() - symbolsToWrap);
        hyphenPos.rx() -=  nearestLetter->boundingRect().width() * nearestLetter->scale() * (1.0 - nearestLetterData.limits.right());
    }

    hyphen->setPos(hyphenPos);

    return hyphen;
}

void SvgView::connectLetters()
{
    if (!connectingLetters)
        return;

    for (int currentWord = 0; currentWord < storedWordItems.size(); currentWord++)
    {
        for (int currentSymbol = 1; currentSymbol < storedWordItems.at(currentWord).size(); currentSymbol++)
        {
            QGraphicsSvgItem * currentLetter = storedWordItems.at(currentWord).at(currentSymbol);
            QGraphicsSvgItem * previousLetter = storedWordItems.at(currentWord).at(currentSymbol - 1);

            //calculate boudingRects; pL means previous letter; cL - current letter
            QSizeF pLBoundingRect, cLBoundingRect;
            pLBoundingRect.setWidth(previousLetter->boundingRect().width() * previousLetter->scale());
            pLBoundingRect.setHeight(previousLetter->boundingRect().height() * previousLetter->scale());
            cLBoundingRect.setWidth(currentLetter->boundingRect().width() * currentLetter->scale());
            cLBoundingRect.setHeight(currentLetter->boundingRect().height() * currentLetter->scale());
            SymbolData pLSymbolData = storedSymbolData.at(currentWord).at(currentSymbol - 1);
            SymbolData cLSymbolData = storedSymbolData.at(currentWord).at(currentSymbol);

            //calculate coordinates of points
            QPointF inPoint, outPoint;
            outPoint.rx() = previousLetter->pos().x() +
                    pLSymbolData.outPoint.x() * pLBoundingRect.width();
            outPoint.ry() = previousLetter->pos().y() +
                    pLSymbolData.outPoint.y() * pLBoundingRect.height();

            inPoint.rx() = currentLetter->pos().x() +
                    cLSymbolData.inPoint.x() * cLBoundingRect.width();
            inPoint.ry() = currentLetter->pos().y() +
                    cLSymbolData.inPoint.y() * cLBoundingRect.height();

            //prepare a pen and draw the line
            QPen pen(fontColor);
            pen.setWidth(penWidth * dpmm);
            pen.setCapStyle(Qt::RoundCap);

            scene->addLine(outPoint.x(), outPoint.y(), inPoint.x(), inPoint.y(), pen);
        }
    }
}

void SvgView::processUnknownSymbol(const QChar &symbol)
{
    switch (symbol.toLatin1())
    {
    case '\t':
        cursor.rx() += wordSpacing * dpmm * spacesInTab - currentLetterSpacing * dpmm;
        break;

    case '\n':
        cursorToNewLine();
        break;

    case ' ':
        cursor.rx() += (wordSpacing - currentLetterSpacing) * dpmm;
        break;

    default:
        cursor.rx() += (fontSize + currentLetterSpacing) * dpmm;
        break;
    }

    storedWordItems.push_back(QVector<QGraphicsSvgItem *>());
    storedSymbolData.push_back(QVector<SymbolData>());
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

    //clear the loaded font
    for (SvgData &data : font.values())
    {
        delete data.renderer;
        data.renderer = nullptr;
    }
    font.clear();

    QString fontDirectory = QFileInfo(fontpath).path() + '/';

    //load the data of symbols except the data for capital (uppercase) letters
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

    //Load uppercase letters.
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

    //start SVG editing
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
    scaleViewBox(svgElement); //scale viewBox to avoid the cut lines with an increase in the width of the line

    //get necessary SVG nodes
    QStringList viewBox = svgElement.attribute("viewBox").split(" ");
    qreal dotsPerUnits = renderer->defaultSize().height() / viewBox.at(3).toDouble();
    qreal newPenWidth = penWidth * dpmm / scale / dotsPerUnits;

    QDomNodeList pathList = doc.elementsByTagName("path");
    QDomNodeList styleList = doc.elementsByTagName("style");

    //change necessary attributes; they can be in "style" nodes or in "path" nodes
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

    //load changed symbol
    renderer->load(doc.toString(0).replace(">\n<tspan", "><tspan").toUtf8());
    font.insert(key, {symbolData, scale, renderer});
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
        attribute.insert(semicolon > endSign ? semicolon : endSign,
                         (attribute.isEmpty() ? "" : ";") + parameter + ":" + newValue);
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
    wordSpacing =   settings.value("word-spacing").toDouble();
    spacesInTab =   settings.value("spaces-in-tab").toInt();
    fontSize =      settings.value("font-size").toDouble();
    penWidth =      settings.value("pen-width").toDouble();
    roundLines =    settings.value("round-lines").toBool();
    useSeed =       settings.value("use-seed").toBool();
    seed =          settings.value("seed").toInt();
    wordWrap =      settings.value("wrap-words").toBool();
    useCustomFontColor = settings.value("use-custom-font-color").toBool();
    connectingLetters =     settings.value("connect-letters").toBool();
    hyphenateWords =     settings.value("hyphenate-words").toBool();

    sheetRect = QRectF(0, 0,
                       settings.value("sheet-width").toInt() * dpmm,
                       settings.value("sheet-height").toInt() * dpmm);

    marginsRect = QRectF(sheetRect.topLeft() + QPointF(settings.value("left-margin").toInt() * dpmm,
                                                       settings.value("top-margin").toInt() * dpmm),
                         sheetRect.bottomRight() - QPointF(settings.value("right-margin").toInt() * dpmm,
                                                           settings.value("bottom-margin").toInt() * dpmm));

    fontColor = QColor(settings.value("font-color").toString());

    leftMarginRandomValue = settings.value("left-margin-random-value").toDouble();
    leftMarginRandomEnabled =  settings.value("left-margin-random-enabled").toBool();
    symbolJumpRandomValue = settings.value("symbol-jump-random-value").toDouble();
    symbolJumpRandomEnabled =  settings.value("symbol-jump-random-enabled").toBool();
    letterSpacingRandomValue = settings.value("letter-spacing-random-value").toDouble();
    letterSpacingRandomEnabled =  settings.value("letter-spacing-random-enabled").toBool();

    markingEnabled = settings.value("marking-enabled").toBool();
    isMarkingLines = settings.value("is-marking-lines").toBool();
    markingColor = QColor(settings.value("marking-color").toString());
    markingCheckSize = settings.value("marking-check-size").toDouble();
    markingLineSize = settings.value("marking-line-size").toDouble();
    markingPenWidth = settings.value("marking-pen-width").toDouble();

    loadFont(settings.value("last-used-font", "Font/DefaultFont.ini").toString());
    settings.endGroup();

    scene->setSceneRect(sheetRect);
    renderText();
}

void SvgView::loadHyphenRules()
{
    hyphenRules.clear();
    //load variables with their values first
    QMap<QString, QString> variables;
    QSettings settings("hyphenationRules.ini", QSettings::IniFormat);
    settings.beginGroup("Variables");

    for (const QString &name : settings.childKeys())
        variables.insert(name, QString::fromUtf8(settings.value(name).toString().toLatin1()));

    //than load rules
    settings.endGroup();
    settings.beginGroup("Rules");

    for (const QString &key : settings.childKeys())
    {
        QString rule = settings.value(key).toString();

        //and replace variables on their values
        for (QString &variable : variables.uniqueKeys())
            rule.replace(variable, variables[variable]);

        hyphenRules.push_back(QRegularExpression(rule));
    }

    settings.endGroup();
}

void SvgView::hideBorders(bool hide)
{
    areBordersHidden = hide;
    scene->items(Qt::AscendingOrder).at(0)->setVisible(!hide); //sheetRect
    scene->items(Qt::AscendingOrder).at(1)->setVisible(!hide); //marginsRect
}

void SvgView::changeLeftRightMargins(bool change)
{
    changeMargins = change;
}

QRectF SvgView::changedVerticalMargins()
{
    QRectF changedMarginsRect;

    if (changeMargins)
        changedMarginsRect = QRectF(QPointF(sheetRect.topRight().x() - marginsRect.topRight().x(),
                                            marginsRect.topLeft().y()),
                                    QPointF(sheetRect.bottomRight().x() - marginsRect.bottomLeft().x(),
                                            marginsRect.bottomRight().y()));
    else
        changedMarginsRect = marginsRect;

    return changedMarginsRect;
}

void SvgView::randomizeMargins()
{
    currentMarginsRect = changedVerticalMargins();

    if (!leftMarginRandomEnabled || leftMarginRandomValue == 0)
        return;

    qreal random = qrand() % uint(leftMarginRandomValue * dpmm);
    if (qrand() % 2)
        random = -random;

    currentMarginsRect.setLeft(currentMarginsRect.x() + (leftMarginRandomValue * dpmm) + random);
}

void SvgView::cursorToNewLine()
{
    randomizeMargins();
    cursor.rx() = currentMarginsRect.x();
    cursor.ry() += (fontSize + lineSpacing) * dpmm;
}

QPointF SvgView::symbolPositionRandomValue()
{
    QPointF randomPos(0.0, 0.0);

    if (!symbolJumpRandomEnabled || symbolJumpRandomValue == 0)
        return randomPos;

    qreal randomY = qrand() % uint(symbolJumpRandomValue * dpmm);
    if (qrand() % 2)
        randomY = -randomY;

    randomPos.setY(randomY);

    return randomPos;
}

void SvgView::randomizeLetterSpacing()
{
    currentLetterSpacing = letterSpacing;
    if (!letterSpacingRandomEnabled || letterSpacingRandomValue == 0)
        return;

    qreal random = qrand() % uint(letterSpacingRandomValue * dpmm);
    if (qrand() % 2)
        random = -random;

    currentLetterSpacing += random;
}

void SvgView::drawMarking()
{
    if (!markingEnabled)
        return;

    qreal width = markingPenWidth * dpmm;
    qreal lineSize = markingLineSize * dpmm;
    qreal checkSize = markingCheckSize * dpmm;
    //y is under the first line of text
    qreal y = marginsRect.top() + fontSize * dpmm + width;

    QPen pen;
    pen.setColor(markingColor);
    pen.setWidth(width);

    if (isMarkingLines)
    {
        for (; y <= currentMarginsRect.bottom(); y += lineSize)
            scene->addLine(0.0, y, sceneRect().right(), y, pen);
    }
    else
    {
        while (y > 0.0)
            y -= checkSize;

        for (; y <= sceneRect().bottom(); y += checkSize)
        {
            scene->addLine(0.0, y - checkSize, sceneRect().right(), y - checkSize, pen);
            scene->addLine(0.0, y, sceneRect().right(), y, pen);
        }

         for (qreal x = sceneRect().left(); x < sceneRect().right(); x += checkSize)
             scene->addLine(x, 0.0, x, sceneRect().bottom(), pen);
    }

    drawMargins = true;

    if (!drawMargins)
        return;

    qreal x;

    if (changeMargins)
        x = scene->items(Qt::AscendingOrder).at(1)->boundingRect().left();
    else
        x = scene->items(Qt::AscendingOrder).at(1)->boundingRect().right();

    pen.setColor(Qt::darkRed);
    pen.setWidth(width * 2);

    scene->addLine(x, 0.0, x, sceneRect().bottom(), pen);
}
