#ifndef SYMBOLDATA_H
#define SYMBOLDATA_H

#include <QtCore/QtCore>

struct SymbolData {
    QString fileName;
    QPointF inPoint;
    QPointF outPoint;
    QRectF limits;
};

Q_DECLARE_METATYPE(SymbolData)
Q_DECLARE_METATYPE(QList<SymbolData>)

QDataStream & operator<<(QDataStream &out, const SymbolData &letter);
QDataStream & operator>>(QDataStream &in, SymbolData &letter);
bool operator==(const SymbolData &left, const SymbolData &right);

#endif // SYMBOLDATA_H
