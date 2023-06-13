#ifndef CHESSALGORITHM_H
#define CHESSALGORITHM_H

#include <QObject>
#include <QPoint>
#include <QHash>
#include "chessboard.h"
#include "uciengine.h"


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

    ChessAlgorithm(QObject *parent = nullptr);

    // Getter method to the board.
    ChessBoard* board() const;

    inline Result result() const { return m_result; }
    inline Player currentPlayer() const { return m_currentPlayer; }

    // Sets the possible player moves, and engine moves.
    void setMoves(int colFrom, int rankFrom);
    void setEngineMoves(int colFrom, int rankFrom);

    QHash<QString, bool> getMoves() const {return m_moves; }

public slots:
    virtual void newGame();
    virtual bool move(int colFrom, int rankFrom, int colTo, int rankTo);
    bool move(const QPoint &from, const QPoint &to);
    QPoint toCoordinates(QString move);

signals:
    void boardChanged(ChessBoard*);
    void endGame(Result);
    void currentPlayerChanged(Player);

protected:
    virtual void setupBoard();
    void setBoard(ChessBoard *board);
    void setResult(Result);
    void setCurrentPlayer(Player);

private:
    Result m_result;
    Player m_currentPlayer;
    QHash<QString, bool> m_moves;
    QHash<QString, bool> m_specialMoves;
    QHash<QString, bool> m_engineMoves;
    UciEngine *m_engine;

    // GamePlay functions.
    void setPawnMoves(char piece, int colFrom, int rankFrom);
    void setKnightMoves(char piece, int colFrom, int rankFrom);
    void setBishopMoves(char piece, int colFrom, int rankFrom);
    void setRookMoves(char piece, int colFrom, int rankFrom);
    void setQueenMoves(char piece, int colFrom, int rankFrom);
    void setKingMoves(char piece, int colFrom, int rankFrom);

    QString toAlgebraic(char piece, int colFrom, int rankFrom, int colTo, int rankTo, bool canTake);

    bool onBoard(int colTo, int rankTo);
};

#endif // CHESSALGORITHM_H
