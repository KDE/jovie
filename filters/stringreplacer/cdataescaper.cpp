#include "cdataescaper.h"
#include <QString>

static const QLatin1String A0("&");
static const QLatin1String B0("&amp;");
static const QLatin1String A1("]]>");
static const QLatin1String B1("]]&gt;");

void cdataEscape(QString* s)
{
    s->replace(A0, B0);
    s->replace(A1, B1);
}

void cdataUnescape(QString* s)
{
    s->replace(B1, A1);
    s->replace(B0, A0);
}
