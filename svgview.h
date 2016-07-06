/*!
    SvgView - class representing a sheet of paper with handwritten text.

    It reads settings from a file, loads current font, changes it, places
    letters on a sheet, connects letters within words and displays the result.
    It also generates a QImage for MainWindow::save* functions.

    An algorithm that places characters on a sheet is described in the function
    renderText(). It takes the QStringRef with text, places as many characters
    as possible on a single sheet and returns the number of the first character
    that function has not processed.

    preventGoingBeyondRightMargin() prevents going beyond right margin
    by wrapping or hypphenating words, or just simply starts a new line.
*/
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
        QSvgRenderer *renderer;
    };

    QGraphicsScene *scene;
    QMultiMap<QChar, SvgData> font;
    QVector<QVector<QGraphicsSvgItem *>> storedWordItems;
    QVector<QVector<SymbolData>> storedSymbolData;
    int dpi;  //dots per inch
    int dpmm; //dots per millimeter
    int spacesInTab;
    int seed;
    bool useCustomFontColor, changeMargins, connectingLetters,
         useSeed, roundLines, wordWrap, hyphenateWords;
    qreal maxScaleFactor, minScaleFactor, currentScaleFactor;
    qreal fontSize, penWidth, letterSpacing, lineSpacing, wordSpacing;
    QRectF sheetRect, marginsRect;
    QColor fontColor;
    QVector<QRegularExpression> hyphenRules;

    SymbolData symbolData, previousSymbolData;
    QRectF currentMarginsRect;
    QSizeF symbolBoundingSize;
    QPointF cursor, previousSymbolCursor; //cursor is pointing to where will be placed
                                          //the top left corner of the next character limits
    qreal previousSymbolWidth;

    void limitScale(qreal factor);  //limited view zoom
    void prepareSceneToRender();
    bool preventGoingBeyondRightMargin(qreal letterWidth, QStringRef text, int currentSymbolIndex);
    void connectLetters();
    void processUnknownSymbol(const QChar &symbol);
    void insertSymbol(QChar key, SymbolData &symbolData);
    void changeAttribute(QString &attribute, QString parameter, QString newValue);
    bool wrapWords(QStringRef text, int currentSymbolIndex);
    bool wrapLastSymbols(int symbolsToWrap);
    bool hyphenate(QStringRef text, int currentSymbolIndex);
    void loadHyphenRules();
    QGraphicsSvgItem * generateHyphen(int symbolsToWrap);
};

#endif // SVGVIEW_H
