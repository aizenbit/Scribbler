#ifndef LETTER_H
#define LETTER_H

#include <QtCore/QtCore>

struct Letter {
    QString fileName;
    QPointF inPoint;
    QPointF outPoint;
    QRectF  limits;
};

Q_DECLARE_METATYPE(Letter);

QDataStream& operator<<(QDataStream& out, const Letter& v);
QDataStream& operator>>(QDataStream& in, Letter& v);

Q_DECLARE_METATYPE(QList<Letter>);

#endif // LETTER_H
