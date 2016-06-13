#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    qRegisterMetaTypeStreamOperators<SymbolData>("SymbolData");
    qRegisterMetaTypeStreamOperators<QList<SymbolData>>("QList<SymbolData>");
    QApplication a(argc, argv);

    QTranslator myTranslator;
    myTranslator.load("Scribbler-" + QLocale::system().name(), "translations");
    a.installTranslator(&myTranslator);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), "translations");
    a.installTranslator(&qtTranslator);

    qApp->addLibraryPath("./plugins");
    MainWindow w;
    w.show();

    return a.exec();
}
