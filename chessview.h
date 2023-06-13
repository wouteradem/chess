#ifndef CHESSVIEW_H
#define CHESSVIEW_H

#include <QWidget>
#include <QPointer>
#include <QList>
#include "chessboard.h"
#include "highlight.h"

// Custom QWidget that represents the chess Board.
// The view renders the current state of the chess board and emits signals of the board.
class ChessView : public QWidget
{
    // Macro.
    Q_OBJECT
    Q_PROPERTY(QSize fieldSize READ fieldSize WRITE setFieldSize NOTIFY fieldSizeChanged)

public:
    // Constructor.
    ChessView(QWidget *parent = nullptr);

    void setBoard(ChessBoard*);

    // Getter method to get the chess board.
    ChessBoard* board() const;

    QSize fieldSize() const;
    void setFieldSize(const QSize &newFielSize);
    QSize sizeHint() const;
    QRect fieldRect(int column, int rank) const;
    QRect fieldCircle(int column, int rank) const;
    QPoint fieldAt(const QPoint &pt) const;

    // Allow to repaint the widget when QEvent::Paint is fired.
    // QPaintEvent contains information about repaint event.
    void paintEvent(QPaintEvent *event);

    // Draw methods.
    void drawField(QPainter *painter, int column, int rank);
    void drawRank(QPainter *painter, int rank);
    void drawColumn(QPainter *painter, int column);

    void setPiece(char type, const QIcon &icon);
    QIcon piece(char type) const;

    void mouseReleaseEvent(QMouseEvent *event);

    // Highlight feature.
    void addHighlight(Highlight *hl);
    void removeHighlight(Highlight *hl);
    inline Highlight *highlight(int index) const
    {
        return m_highlights.at(index);
    }

    inline int highlightCount() const
    {
        return m_highlights.size();
    }
    virtual void drawHighlights(QPainter *painter);

protected:
    void drawPiece(QPainter *painter, int column, int rank);

private:
    // QPointer intialised m_board to NULL.
    QPointer<ChessBoard> m_board;
    QSize m_fieldSize;

    // Key is char and value is the icon.
    QMap<char, QIcon> m_pieces;
    QList<Highlight*> m_highlights;

signals:
    void fieldSizeChanged();
    void clicked(const QPoint &);
};

#endif // CHESSVIEW_H
