#include "manuscript.h"

Manuscript::Manuscript(QObject *parent) : QGraphicsScene(parent)
{
    dpi = 300;
    dpmm = dpi / 25.4;
    sheetSize = QSize(210 * dpmm, 297 * dpmm);
    margins = QSize(200 * dpmm, 287 * dpmm);
    indent = QSize(5 * dpmm, 5 * dpmm);

    this->setSceneRect(0, 0, sheetSize.width(), sheetSize.height());

    this->addText("test text_ test text_");
    this->addRect(0,0,sheetSize.width(), sheetSize.height());

}

Manuscript::~Manuscript()
{

}

void Manuscript::newTemplate()
{

}
