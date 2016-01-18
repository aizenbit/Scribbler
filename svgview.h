#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QApplication>
#include <qmath.h>
#include <QGraphicsSvgItem>
#include <QRegularExpression>
#include <QTextCodec>


class SvgView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit SvgView(QWidget *parent = 0);
    ~SvgView();

signals:

public slots:
    void renderText(QString text = QString());
    QImage renderTextToImage(QString text = QString());
    void loadFont(QString fontpath);
    void loadSettingsFromFile();
    QGraphicsScene * getScene() {return scene;}
    void wheelEvent(QWheelEvent *event);

private:
    qreal maxZoomFactor;
    qreal minZoomFactor;
    qreal currentScaleFactor;
    void limitScale(qreal factor);

    QGraphicsScene * scene;
    int dpi;
    int dpmm; //dots per mm
    int spacesInTab;
    qreal letterSpacing, lineSpacing;
    QRectF sheetRect, marginsRect;

    QMultiMap<QChar, QString> font;
    qreal fontSize;

    bool renderBorders;
};

#endif // SVGVIEW_H
