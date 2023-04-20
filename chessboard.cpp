#include "chessboard.h"

ChessBoard::ChessBoard(int ranks, int columns, QObject *parent)
    : QObject{parent}
{
    m_ranks = ranks;
    m_columns = columns;

    // Once ranks and columns are set, init the board.
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

void ChessBoard::setRanks(int newRanks)
{
    // If ranks is already equal, do nothing.
    if (ranks() == newRanks) return;

    m_ranks = newRanks;
    //initBoard();

    // Emit the signal that ranks has changed.
    emit ranksChanged(m_ranks);
}

void ChessBoard::setColumns(int newColumns)
{
    // If columns is already equal, do nothing.
    if (columns() == newColumns) return;

    m_columns = newColumns;
    //initBoard();

    // Emit the signal that columns has changed.
    emit ranksChanged(m_columns);
}

void ChessBoard::initBoard()
{
    // Fill up the QVector with all spaces.
    m_boardData.fill(' ', ranks() * columns());
}

char ChessBoard::data(int column, int rank) const
{
    // We count the number of ranks and multiply with 8
    // And then we are in the correct rank and only need to add the number of columns.
    // We need to do the -1 because our indexing starts from 0.
    return m_boardData.at((rank - 1) * columns() + (column - 1));
}

void ChessBoard::setData(int column, int rank, char value)
{
    if (setDataInternal(column, rank, value))
    {
        emit dataChanged(column, rank);
    }
}

bool ChessBoard::setDataInternal(int column, int rank, char value)
{
    int index = (rank - 1) * columns() + (column - 1);

    if (m_boardData.at(index) == value) return false;

    m_boardData[index] = value;

    return true;
}

void ChessBoard::movePiece(int fromColumn, int fromRank, int toColumn, int toRank)
{
    setData(toColumn, toRank, data(fromColumn, fromRank));
    setData(fromColumn, fromRank, ' ');
}

void ChessBoard::setFen(const QString &fen)
{
    int index = 0;
    int skip = 0;
    const int columnCount = columns();
    QChar ch;
    for (int rank = ranks(); rank > 0; --rank)
    {
        for (int column = 1; column <= columnCount; ++column)
        {
            if (skip > 0)
            {
                ch = ' ';
                skip--;
            }
            else
            {
                ch = fen.at(index++);
                if (ch.isDigit())
                {
                    skip = ch.toLatin1() - '0';
                    ch = ' ';
                    skip--;
                }
            }
            setDataInternal(column, rank, ch.toLatin1());
        }
        QChar next = fen.at(index++);
        if (next != '/' && next != ' ')
        {
            initBoard();
            return;
        }
    }

    // Update the board!
    emit boardreset();
}
