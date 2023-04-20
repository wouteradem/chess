#ifndef CHESSVIEW_H
#define CHESSVIEW_H

#include <QWidget>
#include <QPointer>
#include <QList>
#include "chessboard.h"

// Class that acts as the UI.
// The view renders the current state of the chess board and emits signals of the board.
// Also signals user interactions like moving a piece, etc.
// This is a QWidget.
class ChessView : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QSize fieldSize READ fieldSize WRITE setFieldSize NOTIFY fieldSizeChanged)

public:
    // Internal class Highlight.
    class Highlight
    {
    public:
        Highlight() {}
        virtual ~Highlight() {}
        virtual int type() const { return 0; }
    };

    explicit ChessView(QWidget *parent = nullptr);
    void setBoard(ChessBoard*);

    // Getter method to get pointer to chess board.
    ChessBoard* board() const;

    QSize fieldSize() const;
    void setFieldSize(const QSize &newFielSize);
    QSize sizeHint() const;
    QRect fieldRect(int column, int rank) const;
    QPoint fieldAt(const QPoint &pt) const;
    void paintEvent(QPaintEvent *event);
    void drawField(QPainter *painter, int column, int rank);
    void drawRank(QPainter *painter, int rank);
    void drawColumn(QPainter *painter, int column);
    void setPiece(char type, const QIcon &icon);
    QIcon piece(char type) const;
    void mouseReleaseEvent(QMouseEvent *event);

    // Highlight.
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
    QMap<char, QIcon> m_pieces;
    QList<Highlight*> m_highlights;

signals:
    void fieldSizeChanged();
    void clicked(const QPoint &);
};

#endif // CHESSVIEW_H
