#include "svgview.h"
    #include <QSettings>
SvgView::SvgView(QWidget *parent) : QGraphicsView(parent)
{
    scaleFactor = 1.0;
    maxZoomFactor = 2.0;
    minZoomFactor = 0.1;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    limitScale(0.3);

    dpi = 300;
    dpmm = dpi / 25.4;
    sheetRect = QRectF(0, 0, 210 * dpmm, 297 * dpmm);
    marginsRect = QRectF(5 * dpmm, 5 * dpmm, 200 * dpmm, 287 * dpmm);
    fontSize = 6.0;
    letterSpacing = -10.0;

    scene = new QGraphicsScene(sheetRect);
    scene->addRect(sheetRect);
    scene->addRect(marginsRect, QPen(Qt::darkGray));

    //for testing
    /*font.insert('a', "E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_A.svg");
    font.insert('a', "E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_A2.svg");
    font.insert('b', "E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_B.svg");
    font.insert('c', "E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_C.svg");*/
    setScene(scene);

    QList<QString> aFiles, bFiles;
    aFiles.push_back("E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_A.svg");
    aFiles.push_back("E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_A2.svg");
    bFiles.push_back("E:\\Qt\\Projects\\Scribbler\\resources\\Font\\Letter_B.svg");
    QSettings fontSettings("DefaultFont.ini", QSettings::IniFormat);
    fontSettings.beginGroup("Font");
    fontSettings.setValue(QString("ы"), QVariant(aFiles));
    fontSettings.setValue(QString("b"), QVariant(bFiles));
    fontSettings.endGroup();
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
    qreal newFactor = scaleFactor * factor;
    if (newFactor < maxZoomFactor && newFactor > minZoomFactor)
    {
        scaleFactor = newFactor;
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
    fontDirectory.remove(QRegularExpression("\\w+.ini$"));

    font.clear();
    for (QString & key : fontSettings.allKeys())
        for (QString & value : fontSettings.value(key).toStringList())
            font.insert(key[0], fontDirectory + value);

    fontSettings.endGroup();
}
