#include "svgview.h"
    #include <QSettings>
SvgView::SvgView(QWidget *parent) : QGraphicsView(parent)
{
    currentScaleFactor = 1.0;
    maxZoomFactor = 3.0;
    minZoomFactor = 0.05;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    limitScale(0.3);

    //dpi = 300;
    //dpmm = dpi / 25.4;
    //sheetRect = QRectF(0, 0, 210 * dpmm, 297 * dpmm);

    scene = new QGraphicsScene();

    loadSettingsFromFile();

    setScene(scene);
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
    if (newFactor < maxZoomFactor && newFactor > minZoomFactor)
    {
        currentScaleFactor = newFactor;
        scale(factor, factor);
    }
}

void SvgView::renderText(QString text)
{
    scene->clear();

    scene->addRect(sheetRect);
    scene->addRect(marginsRect, QPen(Qt::darkGray));

    QPointF cursor(marginsRect.x(), marginsRect.y() );
    for (QChar symbol : text)
    {
        qreal letterWidth = fontSize * dpmm, letterHeight = fontSize * dpmm;

        if (cursor.x() > (marginsRect.x() + marginsRect.width() - letterWidth))
            cursor += QPointF(marginsRect.x() - cursor.x(), letterHeight);

        if (!font.contains(symbol))
        {
            cursor += QPointF(letterWidth, 0.0);
            continue;
        }

        QGraphicsSvgItem * letter = new QGraphicsSvgItem(font.values(symbol).at(qrand() % font.values(symbol).size()));

        letter->setScale(letterHeight / letter->boundingRect().height());
        letterWidth = letter->boundingRect().width() * letter->scale() + letterSpacing;
        letter->setPos(cursor);
        cursor += QPointF(letterWidth, 0.0);


        scene->addItem(letter);
    }
}

void SvgView::loadFont(QString fontpath)
{
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
    for (QString & key : fontSettings.allKeys())
        for (QString & value : fontSettings.value(key).toStringList())
            font.insert(key[0].toLower(), fontDirectory + value);

    //It's a dirty hack, which helps to distinguish uppercase and lowercase
    //letters on freaking case-insensetive Windows
    fontSettings.beginGroup("UpperCase");
    for (QString & key : fontSettings.allKeys())
        for (QString & value : fontSettings.value(key).toStringList())
            font.insert(key[0].toUpper(), fontDirectory + value);
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
    fontSize = settings.value("font-size").toDouble();
    sheetRect = QRectF(0, 0,
                       settings.value("sheet-width").toInt() * dpmm,
                       settings.value("sheet-height").toInt() * dpmm);

    marginsRect = QRectF(sheetRect.topLeft() + QPointF(settings.value("left-margins").toInt() * dpmm,
                                                       settings.value("top-margins").toInt() * dpmm),
                         sheetRect.bottomRight() - QPointF(settings.value("bottom-margins").toInt() * dpmm,
                                                           settings.value("right-margins").toInt() * dpmm));
    settings.endGroup();

    scene->setSceneRect(sheetRect);
    renderText();
}
