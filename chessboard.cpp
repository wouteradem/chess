#include "chessboard.h"
#include <QDebug>

/*
 * Datastructure that contains the piece data and allows for piece movement.
 */
ChessBoard::ChessBoard(int ranks, int columns, QObject *parent)
    : QObject{parent}
{
    m_ranks = ranks;
    m_columns = columns;
    m_nrOfMoves = 0;

    // Once ranks and columns are set, make an empty board.
    initBoard();
}

int ChessBoard::ranks() const
{
    return m_ranks;
}

int ChessBoard::columns() const
{
    return m_columns;
}

int ChessBoard::nrOfMoves() const
{
    return m_nrOfMoves;
}

void ChessBoard::setRanks(int newRanks)
{
    // If ranks is already equal, do nothing.
    if (ranks() == newRanks) return;

    m_ranks = newRanks;

    // Not 100% sure if that's really needed.
    initBoard();

    // Emit signal that ranks has changed.
    emit ranksChanged(m_ranks);
}

void ChessBoard::setColumns(int newColumns)
{
    // If columns is already equal, do nothing.
    if (columns() == newColumns) return;

    m_columns = newColumns;

    // Not 100% sure if that's really needed.
    initBoard();

    // Emit signal that columns has changed.
    emit columnsChanged(m_columns);
}

void ChessBoard::setNrOfMoves(int nr)
{
    if (nrOfMoves() == nr) return;

    m_nrOfMoves++;

    // Emit signal that number of moves has changed.
    emit nrOfMovesChanged(m_nrOfMoves);
}

/*
 * One dimensional QVector of chars that gets filled to empty board.
 * No pieces are placed yet!
 */
void ChessBoard::initBoard()
{
    m_boardData.fill(' ', ranks() * columns());
}

/*
 * Returns position on the board.
 * Based on return value of character we are able to identify a chess piece.
 */
char ChessBoard::data(int column, int rank) const
{
   /**
    *    -----------------
    *  8 | | | | | | | | |
    *    -----------------
    *  7 | | | | | | | | |
    *    -----------------
    *  6 | | | | | | | | |
    *    -----------------
    *  5 | | | | | | | | |
    *    -----------------
    *  4 | | | | |X| | | |
    *    -----------------
    *  3 | | | | | | | | |
    *    -----------------
    *  2 | | | | | | | | |
    *    -----------------
    *  1 | | | | | | | | |
    *    -----------------
    *     a b c d e f g h
    */
    return m_boardData.at((rank - 1) * columns() + (column - 1));
}

/*
 * Sets chess piece on postion at (rank, column)
 * and emits that data change.
 */
void ChessBoard::setData(int column, int rank, char value)
{
    if (setDataInternal(column, rank, value))
    {
        emit dataChanged(column, rank);
    }
}

/*
 * Helper function that returns true if piece can be moved to
 * given (rank, column).
 */
bool ChessBoard::setDataInternal(int column, int rank, char value)
{
    int index = (rank - 1) * columns() + (column - 1);

    // Check if we can actually put the piece there.
    if (m_boardData.at(index) == value)
        return false;

    m_boardData[index] = value;

    return true;
}

/*
 * Moves a chess piece and sets the previous value to a space.
 */
void ChessBoard::movePiece(int fromColumn, int fromRank, int toColumn, int toRank)
{
    setData(toColumn, toRank, data(fromColumn, fromRank));
    setData(fromColumn, fromRank, ' ');
}

/*
 * Helper function that sets the pieces on the board
 * according to the FEN code.
 * https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
 *
 * Example: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
 *
 *    -----------------
 *  8 |r|n|b|q|k|b|n|r|
 *    -----------------
 *  7 |p|p|p|p|p|p|p|p|
 *    -----------------
 *  6 | | | | | | | | |
 *    -----------------
 *  5 | | | | | | | | |
 *    -----------------
 *  4 | | | | | | | | |
 *    -----------------
 *  3 | | | | | | | | |
 *    -----------------
 *  2 |P|P|P|P|P|P|P|P|
 *    -----------------
 *  1 |R|N|B|Q|K|B|N|R|
 *    -----------------
 *     a b c d e f g h
 *
 * Got some inspiration from: https://codereview.stackexchange.com/questions/251795/parsing-a-chess-fen
 */
void ChessBoard::setFen(const QString &fen)
{
    int index = 0;
    int skip = 0;
    const int columnCount = columns();
    QChar ch;

    // Start from top left a8 to h8 and go to h1.
    for (int rank = ranks(); rank > 0; --rank)
    {
        for (int column = 1; column <= columnCount; ++column)
        {
            // Matches a number (e.g. 8 in the example).
            if (skip > 0)
            {
                ch = ' ';
                skip--;
            }
            else
            {
                // Read the character.
                ch = fen.at(index++);
                if (ch.isDigit())
                {
                    // Convert character to an integer.
                    skip = ch.toLatin1() - '0';
                    ch = ' ';
                    skip--;
                }
            }
            setDataInternal(column, rank, ch.toLatin1());
        }

        // Skip the '/' and ' ' characters.
        QChar next = fen.at(index++);
        if (next != '/' && next != ' ')
        {
            qDebug() << "Bail out char = " << next;

            initBoard();
            return;
        }
    }

    // Emit signal that the board is set.
    emit boardreset();
}
