#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QtCore/QRegularExpression>
#include <QtCore/QTextCodec>
#include <QtCore/QSettings>
#include <QtCore/QtMath>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsColorizeEffect>
#include <QtWidgets/QApplication>
#include <QtGui/QWheelEvent>
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <QtXml/QDomDocument>

#include "symboldata.h"

class SvgView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SvgView(QWidget *parent = 0);
    ~SvgView();

    static void scaleViewBox(QDomElement &svgElement);
    static const qreal &scaleCanvasValueRef;

public slots:
    int renderText(const QStringRef &text = QStringRef());
    QImage saveRenderToImage();
    void loadFont(QString fontpath = QString());
    void loadSettingsFromFile();
    void hideBorders(bool hide);
    void changeLeftRightMargins(bool change);
    QList<QChar> getFontKeys() {return font.uniqueKeys();}

protected:
    void wheelEvent(QWheelEvent *event);

private:
    struct SvgData
    {
        SymbolData symbolData;
        qreal scale;
        qreal width;
        QSvgRenderer *renderer;
    };

    QGraphicsScene *scene;
    QMultiMap<QChar, SvgData> font;
    QSet<QChar> specialSymbols;
    int dpi;  //dots per inch
    int dpmm; //dots per millimeter
    int spacesInTab;
    int seed;
    bool useCustomFontColor, changeMargins, connectLetters,
         useSeed, roundLines;
    qreal maxScaleFactor, minScaleFactor, currentScaleFactor;
    qreal fontSize, penWidth, letterSpacing, lineSpacing;
    QRectF sheetRect, marginsRect;
    QColor fontColor;

    static qreal scaleCanvasValue;
    static bool scaleCanvas;

    SymbolData symbolData, previousLetterData;
    QRectF currentMarginsRect;
    QSizeF symbolBoundingSize;
    QPointF cursor, previousLetterCursor;
    QGraphicsSvgItem *lastLetter;

    void limitScale(qreal factor);  //limited view zoom
    void prepareSceneToRender();
    void preventGoingBeyondRightMargin(qreal letterWidth);
    void connectLastLetterToCurrent();
    void processUnknownSymbol(const QChar &symbol);
    void insertSymbol(QChar key, SymbolData &symbolData);
    void changeAttribute(QString &attribute, QString parameter, QString newValue);
};

#endif // SVGVIEW_H
