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

#include "letter.h"

class SvgView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SvgView(QWidget *parent = 0);
    ~SvgView();

public slots:
    QGraphicsScene * getScene() {return scene;}

    int renderText(const QStringRef &text = QStringRef());
    QImage saveRenderToImage();
    void loadFont(QString fontpath = QString());
    void loadSettingsFromFile();
    void hideBorders(bool hide);
    void changeLeftRightMargins(bool change);

protected:
    void wheelEvent(QWheelEvent *event);

private:
    QGraphicsScene *scene;
    QMultiMap<QChar, Letter> font;
    int dpi;  //dots per inch
    int dpmm; //dots per millimeter
    int spacesInTab;
    bool useCustomFontColor, changeMargins, connectLetters;
    qreal maxScaleFactor, minScaleFactor, currentScaleFactor;
    qreal fontSize, letterSpacing, lineSpacing;
    QRectF sheetRect, marginsRect;
    QColor fontColor;

    Letter letterData, previousLetterData;
    QRectF currentMarginsRect;
    QSize letterBoundingRect;
    QPointF cursor, previousLetterCursor;
    QGraphicsSvgItem *lastLetter;

    void limitScale(qreal factor);  //limited view zoom
    void prepareSceneToRender();
    void preventGoingBeyondRightMargin();
    void connectLastLetterToCurrent();
};

#endif // SVGVIEW_H
