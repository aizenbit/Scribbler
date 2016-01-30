#include "svgview.h"

SvgView::SvgView(QWidget *parent) : QGraphicsView(parent)
{
    currentScaleFactor = 1.0;
    maxZoomFactor = 3.0;
    minZoomFactor = 0.05;
    changeMargins = false;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
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
    qreal newFactor = currentScaleFactor *factor;
    if (newFactor < maxZoomFactor && newFactor > minZoomFactor)
    {
        currentScaleFactor = newFactor;
        scale(factor, factor);
    }
}

int SvgView::renderText(const QStringRef &text)
{
    scene->clear();

    QRectF currentMarginsRect;

    if (changeMargins)
        currentMarginsRect = QRectF(QPointF(sheetRect.topRight().x() - marginsRect.topRight().x(),
                                            marginsRect.topLeft().y()),
                                    QPointF(sheetRect.bottomRight().x() - marginsRect.bottomLeft().x(),
                                            marginsRect.bottomRight().y()));
    else
        currentMarginsRect = marginsRect;

    scene->addRect(sheetRect);
    scene->addRect(currentMarginsRect, QPen(Qt::darkGray));

    QPointF cursor(currentMarginsRect.x(), currentMarginsRect.y());
    int endOfSheet = 0;

    for (QChar symbol : text)
    {
        qreal letterWidth = fontSize * dpmm / 4, letterHeight = fontSize * dpmm;

        //don't try to go beyond the right margin
        if (cursor.x() > (currentMarginsRect.x() + currentMarginsRect.width() - letterWidth))
        {
            if (symbol.isSpace())   //ignore whitespace-symbols at the end of the current line
            {
                endOfSheet++;
                continue;
            }
            cursor += QPointF(currentMarginsRect.x() - cursor.x(), letterHeight + lineSpacing * dpmm);
        }

        //stop rendering when you reach the end of sheet
        if (cursor.y() > currentMarginsRect.bottomRight().y() - letterHeight)
            return endOfSheet;

        if (symbol.isSpace())
        {
            switch (symbol.toLatin1())
            {
            case '\t':
                cursor += QPointF(letterWidth * spacesInTab, 0.0);
                endOfSheet++;
                continue;

            case '\n':
                cursor += QPointF(currentMarginsRect.x() - cursor.x(), letterHeight + lineSpacing * dpmm);
                endOfSheet++;
                continue;

            default:
                cursor += QPointF(letterWidth, 0.0);
                endOfSheet++;
                continue;
            }
        }

        if (!font.contains(symbol))
        {
            cursor += QPointF(letterWidth, 0.0);
            endOfSheet++;
            continue;
        }

        Letter letterData = font.values(symbol).at(qrand() % font.values(symbol).size());

        QGraphicsSvgItem *letter = new QGraphicsSvgItem(letterData.fileName);

        if (useCustomFontColor)
        {
            QGraphicsColorizeEffect *colorEffect = new QGraphicsColorizeEffect();
            colorEffect->setColor(fontColor);
            letter->setGraphicsEffect(colorEffect);
        }

        letter->setScale(letterHeight / letter->boundingRect().height());
        letterWidth = letter->boundingRect().width() * letter->scale() + letterSpacing * dpmm;
        letter->setPos(cursor);
        cursor += QPointF(letterWidth, 0.0);

        scene->addItem(letter);
        endOfSheet++;
    }

    return endOfSheet;
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

    QString fontDirectory = fontpath;
    fontDirectory.remove(QRegularExpression("\\w+.\\w+$"));

    font.clear();
    for (const QString &key : fontSettings.childKeys())
        for (Letter &value : fontSettings.value(key).value<QList<Letter>>())
        {
            value.fileName = fontDirectory + value.fileName;
            font.insert(key.at(0).toLower(), value);
        }

    //It's a dirty hack, which helps to distinguish uppercase and lowercase
    //letters on freaking case-insensetive Windows
    fontSettings.beginGroup("UpperCase");
    for (const QString &key : fontSettings.childKeys())
        for (Letter &value : fontSettings.value(key).value<QList<Letter>>())
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

    loadFont(settings.value("last-used-font", "\\Font\\DefaultFont.ini").toString());

    fontColor = QColor(settings.value("font-color").toString());
    useCustomFontColor = settings.value("use-custom-font-color").toBool();

    settings.endGroup();

    scene->setSceneRect(sheetRect);
    renderText();
}

void SvgView::hideBorders(bool hide)
{
    scene->items(Qt::AscendingOrder).at(0)->setVisible(!hide);
    scene->items(Qt::AscendingOrder).at(1)->setVisible(!hide);
}

void SvgView::changeLeftRightMargins(bool change)
{
    changeMargins = change;
}
