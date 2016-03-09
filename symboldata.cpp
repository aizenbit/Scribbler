#include "symboldata.h"

QDataStream & operator<<(QDataStream &out, const SymbolData &data)
{
    out << data.fileName << data.inPoint
        << data.outPoint << data.limits;
    return out;
}

QDataStream & operator>>(QDataStream &in, SymbolData &data)
{
    in >> data.fileName;
    in >> data.inPoint;
    in >> data.outPoint;
    in >> data.limits;
    return in;
}

bool operator==(const SymbolData &left, const SymbolData &right)
{
    if (left.fileName == right.fileName
            && left.inPoint == right.inPoint
            && left.outPoint == right.outPoint
            && left.limits == right.limits)
        return true;
    else
        return false;
}
