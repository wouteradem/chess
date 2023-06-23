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
    Q_PROPERTY(UciEngine engine READ engine CONSTANT)
    Q_PROPERTY(Player currentPlayer READ currentPlayer NOTIFY currentPlayerChanged)
    Q_PROPERTY(QString currentMove READ currentMove WRITE setCurrentMove NOTIFY currentMoveChanged)

    ChessBoard *m_board;

public:
    enum Result {NoResult, WhiteWin, BlackWin, Draw, StaleMate};
    Q_ENUM(Result)

    enum Player {NoPlayer, WhitePlayer, BlackPlayer};
    Q_ENUM(Player)

    ChessAlgorithm(QObject *parent = nullptr);
    ~ChessAlgorithm();

    // Getter method to the board.
    ChessBoard* board() const;

    inline Result result() const { return m_result; }
    inline UciEngine* engine() const { return m_engine; }
    inline Player currentPlayer() const { return m_currentPlayer; }
    inline QString currentMove() const { return m_currentMove; }

    // Sets the possible player moves, and engine moves.
    void setMoves(int colFrom, int rankFrom);
    void setEngineMoves(QString fen);
    QString getFENBoard();
    bool check(QString player);

    QHash<QString, bool> getMoves() const {return m_moves;}

public slots:
    virtual void newGame();
    virtual bool move(int colFrom, int rankFrom, int colTo, int rankTo);
    bool move(const QPoint &from, const QPoint &to);
    QPoint toCoordinates(QString move);
    void whiteCastle(ChessBoard::CastleType);

signals:
    void boardChanged(ChessBoard*);
    void gameOver(ChessAlgorithm::Result);
    void checked(QPoint);
    void unChecked();
    void currentPlayerChanged(Player);
    void currentMoveChanged(QString);

protected:
    virtual void setupBoard();
    void setBoard(ChessBoard *board);
    void setResult(Result);
    void setCurrentPlayer(Player);
    void setCurrentMove(QString);

private:
    Result m_result;
    Player m_currentPlayer;
    QString m_currentMove;
    QHash<QString, bool> m_moves;
    QHash<QString, bool> m_engineMoves;
    UciEngine *m_engine;
    bool m_check;
    bool m_whiteCastling;

    // GamePlay functions.
    void setPawnMoves(QChar piece, int colFrom, int rankFrom);
    void setKnightMoves(QChar piece, int colFrom, int rankFrom);
    void setBishopMoves(QChar piece, int colFrom, int rankFrom);
    void setRookMoves(QChar piece, int colFrom, int rankFrom);
    void setQueenMoves(QChar piece, int colFrom, int rankFrom);
    void setKingMoves(QChar piece, int colFrom, int rankFrom);

    QString toAlgebraic(QChar piece, int colFrom, int rankFrom, int colTo, int rankTo, bool canTake);

    bool onBoard(int colTo, int rankTo);
};

#endif // CHESSALGORITHM_H
