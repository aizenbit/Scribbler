#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    qRegisterMetaTypeStreamOperators<Letter>("Letter");
    qRegisterMetaTypeStreamOperators<QList<Letter>>("QList<Letter>");
    QApplication a(argc, argv);

    QTranslator myTranslator;
    myTranslator.load("Scribbler-" + QLocale::system().name());
    a.installTranslator(&myTranslator);

    qApp->addLibraryPath("./plugins");

    MainWindow w;
    w.show();

    return a.exec();
}
