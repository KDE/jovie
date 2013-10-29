#ifndef TESTCDATAESCAPER_H
#define TESTCDATAESCAPER_H

#include <QObject>

class TestCDATAEscaper : public QObject
{
    Q_OBJECT

private slots:
    void escape();
    void unescape();
    void roundtrip();
};

#endif // TESTCDATAESCAPER_H
