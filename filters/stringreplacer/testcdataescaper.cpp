#include <QtTest>
#include "testcdataescaper.h"
#include "cdataescaper.h"

static const QString original(
        QString::fromAscii("foo & bar ]]> baz")
        );
static const QString escaped(
        QString::fromAscii("foo &amp; bar ]]&gt; baz")
        );

void TestCDATAEscaper::escape()
{
    QString s(original);
    cdataEscape(&s);
    QCOMPARE(s, escaped);
}

void TestCDATAEscaper::unescape()
{
    QString s(escaped);
    cdataUnescape(&s);
    QCOMPARE(s, original);
}

void TestCDATAEscaper::roundtrip()
{
    QString s(original);
    cdataEscape(&s);
    cdataUnescape(&s);
    QCOMPARE(s, original);
}

QTEST_MAIN(TestCDATAEscaper)
#include "testcdataescaper.moc"
