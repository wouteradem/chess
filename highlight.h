#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include <QPainter>
#include <QList>

class Highlight
{
public:
    Highlight() {}
    virtual ~Highlight() {}
    virtual int type() const { return 0; }
};

#endif // HIGHLIGHT_H
