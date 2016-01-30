#include "letter.h"

QDataStream & operator<<(QDataStream &out, const Letter &letter)
{
    out << letter.fileName << letter.inPoint
        << letter.outPoint << letter.limits;
    return out;
}

QDataStream & operator>>(QDataStream &in, Letter &letter)
{
    in >> letter.fileName;
    in >> letter.inPoint;
    in >> letter.outPoint;
    in >> letter.limits;
    return in;
}
