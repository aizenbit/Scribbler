#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QApplication>
#include <qmath.h>
#include <QGraphicsSvgItem>
#include <QRegularExpression>
#include <QTextCodec>
#include <QSettings>
#include <QGraphicsColorizeEffect>

class SvgView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit SvgView(QWidget *parent = 0);
    ~SvgView();

signals:

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
    QGraphicsScene * scene;
    QMultiMap<QChar, QString> font;
    int dpi;  //dots per inch
    int dpmm; //dots per millimeter
    int spacesInTab;
    bool useCustomFontColor, changeMargins;
    qreal maxZoomFactor, minZoomFactor, currentScaleFactor;
    qreal fontSize, letterSpacing, lineSpacing;
    QRectF sheetRect, marginsRect;
    QColor fontColor;

    void limitScale(qreal factor);  //limited view zoom
};

#endif // SVGVIEW_H
