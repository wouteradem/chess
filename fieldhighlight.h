#ifndef FIELDHIGHLIGHT_H
#define FIELDHIGHLIGHT_H

#include "highlight.h"

class FieldHighlight: public Highlight
{
public:
    enum HighLightType {
        Rectangle = 1,
        Circle = 2,
    };

    FieldHighlight(int column, int rank, QColor color, HighLightType type): m_field(column, rank), m_color(color), m_type(type) {}

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
        return m_type;
    }

private:
    QPoint m_field;
    QColor m_color;
    HighLightType m_type;
};

#endif // FIELDHIGHLIGHT_H
