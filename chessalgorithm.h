#ifndef CHESSALGORITHM_H
#define CHESSALGORITHM_H

#include <QObject>
#include "chessboard.h"


// Chess Algorithm that drives the game.
class ChessAlgorithm : public QObject
{
    Q_OBJECT

    ChessBoard *m_board;

public:
    explicit ChessAlgorithm(QObject *parent = nullptr);

    // Getter method to the board.
    ChessBoard* board() const;

public slots:
    // Allow other chess variants to overwrite in derived class.
    virtual void newGame();

signals:
    // Broadcast whenever the board has changed.
    void boardChanged(ChessBoard*);

protected:
    // Allow other chess variants to overwrite in derived class.
    virtual void setupBoard();
    void setBoard(ChessBoard *board);
};

#endif // CHESSALGORITHM_H
