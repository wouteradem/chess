#include "chessalgorithm.h"
#include "chessboard.h"
#include <QDebug>

ChessAlgorithm::ChessAlgorithm(QObject *parent)
    : QObject{parent}
{
    // Initialise the board to null pointer.
    m_board = nullptr;

    // Start the game with no player and no result set.
    m_currentPlayer = NoPlayer;
    m_result = NoResult;
}

ChessBoard* ChessAlgorithm::board() const
{
    return m_board;
}

void ChessAlgorithm::setBoard(ChessBoard *board)
{
    // If board is already set, then do nothing.
    if (board == m_board) return;

    // If private member board already exists remove it and reset it.
    if (m_board) delete m_board;
    m_board = board;

    // Finally, emit that the new board is set.
    emit boardChanged(m_board);
}

void ChessAlgorithm::setupBoard()
{
    // Setup a classical chess board with 8 ranks and 8 columns.
    setBoard(new ChessBoard(8, 8, this));
}

void ChessAlgorithm::newGame()
{
    // First, setup the board.
    setupBoard();

    // Secondly, set the pieces on the board in FEN notation.
    // Reference: https://www.chess.com/terms/fen-chess.
    // First character 'w' after the board setup determines the player to start.
    board()->setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    setResult(NoResult);

    // White always takes the first move.
    // We could have other chess variants. In that case we'd need to subclass from ChessAlgorithm.
    setCurrentPlayer(WhitePlayer);
}

void ChessAlgorithm::setResult(Result value)
{
    if (result() == value)
        return;

    if (result() == NoResult)
    {
        m_result = value;
        emit endGame(m_result);
    }
    else
    {
        m_result = value;
    }
}

void ChessAlgorithm::setCurrentPlayer(Player value)
{
    if (currentPlayer() == value)
        return;

    m_currentPlayer = value;
    emit currentPlayerChanged(m_currentPlayer);
}

bool ChessAlgorithm::move(int colFrom, int rankFrom, int colTo, int rankTo)
{
    /*
    // Prevent compile time warnings.
    Q_UNUSED(colFrom)
    Q_UNUSED(rankFrom)
    Q_UNUSED(colTo)
    Q_UNUSED(rankTo)

    return false;
    */

    if (currentPlayer() == NoPlayer)
        return false;

    // Get the piece that moves.
    char source = board()->data(colFrom, rankFrom);

    // Check which piece is moved.
    if (currentPlayer() == WhitePlayer)
    {
        switch(source)
        {
        case 'P':
            qDebug() << "White pawn moves";
            if (checkMove(source, colFrom, rankFrom, colTo, rankTo))
            {
                board()->movePiece(colFrom, rankFrom, colTo, rankTo);
            }
            else
            {
                qDebug() << "Illegal move";
            }
            break;
        }
    }
    else if (m_currentPlayer == BlackPlayer)
    {
        switch(source)
        {
        case 'p':
            qDebug() << "Black pawn moves";
            break;
        }
    }

    return true;
}

bool ChessAlgorithm::move(const QPoint &from, const QPoint &to)
{
    return move(from.x(), from.y(), to.x(), to.y());
}

bool ChessAlgorithm::checkMove(char source, int colFrom, int rankFrom, int colTo, int rankTo)
{
    // Check if the move is in the list of available moves.
    return true;
}

void ChessAlgorithm::setPossibleMoves(int colFrom, int rankFrom)
{
    char source = board()->data(colFrom, rankFrom);
    qDebug() << "Getting possible moves for piece " << source;

    switch (source)
    {
    case 'P':
        qDebug() << "Moving pawn";
        break;
    }
}
