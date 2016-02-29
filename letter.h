#ifndef LETTER_H
#define LETTER_H

#include <QtCore/QtCore>

struct Letter {
    QString fileName;
    QPointF inPoint;
    QPointF outPoint;
    QRectF  limits;
};

Q_DECLARE_METATYPE(Letter)
Q_DECLARE_METATYPE(QList<Letter>)

QDataStream & operator<<(QDataStream &out, const Letter &letter);
QDataStream & operator>>(QDataStream &in, Letter &letter);
bool operator==(const Letter &left, const Letter &right);

#endif // LETTER_H
