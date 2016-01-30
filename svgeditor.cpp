#include "svgeditor.h"

SvgEditor::SvgEditor(QWidget *parent) : QSvgWidget(parent)
{
    QPalette svgPalette = this->palette();
    svgPalette.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(svgPalette);
}

void SvgEditor::load(const QString & file)
{
    QSvgWidget::load(file);
    QSize letterSize = renderer()->defaultSize();
    setFixedWidth(letterSize.width() * static_cast<qreal>(height()) / letterSize.height());
}
