#include "letter.h"

QDataStream& operator<<(QDataStream& out, const Letter& v)
{
    out << v.fileName << v.inPoint << v.outPoint << v.limits;
    return out;
}

QDataStream& operator>>(QDataStream& in, Letter& v)
{
    in >> v.fileName;
    in >> v.inPoint;
    in >> v.outPoint;
    in >> v.limits;
    return in;
}
