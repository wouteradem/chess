#ifndef FIELDHIGHLIGHT_H
#define FIELDHIGHLIGHT_H

#include "highlight.h"

class FieldHighlight: public Highlight
{
public:
    enum {
        Type = 1
    };

    FieldHighlight(int column, int rank, QColor color): m_field(column, rank), m_color(color) {}

    inline int column() const
    {
        return m_field.x();
    }

    inline int rank() const
    {
        return m_field.y();
    }

    inline QColor color() const
    {
        return m_color;
    }

    int type() const
    {
        return Type;
    }

private:
    QPoint m_field;
    QColor m_color;
};

#endif // FIELDHIGHLIGHT_H
