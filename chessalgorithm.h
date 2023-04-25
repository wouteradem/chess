#ifndef CHESSALGORITHM_H
#define CHESSALGORITHM_H

#include <QObject>
#include <QPoint>
#include "chessboard.h"


// Chess Algorithm that drives the game.
class ChessAlgorithm : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Result result READ result CONSTANT)
    Q_PROPERTY(Player currentPlayer READ currentPlayer NOTIFY currentPlayerChanged)

    ChessBoard *m_board;

public:
    enum Result {NoResult, WhiteWin, BlackWin, Draw, StaleMate};
    Q_ENUM(Result)

    enum Player {NoPlayer, WhitePlayer, BlackPlayer};
    Q_ENUM(Player)

    explicit ChessAlgorithm(QObject *parent = nullptr);

    // Getter method to the board.
    ChessBoard* board() const;

    inline Result result() const { return m_result; }
    inline Player currentPlayer() const { return m_currentPlayer; }
    void setPossibleMoves(int colFrom, int rankFrom);

public slots:
    // Allow other chess variants to overwrite in derived class.
    virtual void newGame();
    virtual bool move(int colFrom, int rankFrom, int colTo, int rankTo);
    bool move(const QPoint &from, const QPoint &to);

signals:
    // Broadcast whenever the board has changed.
    void boardChanged(ChessBoard*);
    void endGame(Result);
    void currentPlayerChanged(Player);

protected:
    // Allow other chess variants to overwrite in derived class.
    virtual void setupBoard();
    void setBoard(ChessBoard *board);
    void setResult(Result);
    void setCurrentPlayer(Player);
    virtual bool checkMove(char source, int colFrom, int rankFrom, int colTo, int rankTo);

private:
    Result m_result;
    Player m_currentPlayer;
};

#endif // CHESSALGORITHM_H
