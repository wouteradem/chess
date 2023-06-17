#include "chessboard.h"
#include <QDebug>
#include <QPoint>

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

bool ChessBoard::whiteChecked() const
{
    return m_whiteChecked;
}

bool ChessBoard::blackChecked() const
{
    return m_blackChecked;
}


ChessBoard::CastleType ChessBoard::whiteCastled() const
{
    return m_whiteCastled;
}

ChessBoard::CastleType ChessBoard::blackCastled() const
{
    return m_blackCastled;
}

void ChessBoard::setWhiteChecked(bool isChecked)
{
    m_whiteChecked = isChecked;
}

void ChessBoard::setBlackChecked(bool isChecked)
{
    qInfo() << Q_FUNC_INFO;

    m_blackChecked = isChecked;
}

void ChessBoard::setWhiteCastled(ChessBoard::CastleType type)
{
    if (type == CastleType::Long)
    {
        m_whiteCastled = CastleType::Long;
    }
    else if (type == CastleType::Short)
    {
        m_whiteCastled = CastleType::Short;
    }

    emit whiteHasCastled(type);
}

void ChessBoard::setBlackCastled(ChessBoard::CastleType type)
{
    if (type == CastleType::Long)
    {
        m_blackCastled = CastleType::Long;
        movePiece(1, 8, 4, 8);
    }
    else if (type == CastleType::Short)
    {
        m_blackCastled = CastleType::Short;
        movePiece(8, 8, 6, 8);
    }

    emit blackHasCastled(type);
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
    m_nrOfMoves += nr;

    // Emit signal that number of moves has changed.
    emit nrOfMovesChanged("e4");
}

/*
 * One dimensional QVector of chars that gets filled to empty board.
 * No pieces are placed yet!
 */
void ChessBoard::initBoard()
{
    m_boardData.fill(' ', ranks() * columns());

    emit boardReset();
}

/*
 * Returns position on the board.
 * Based on return value of character we are able to identify a chess piece.
 */
QChar ChessBoard::data(int column, int rank) const
{
    return m_boardData.at((rank - 1) * columns() + (column - 1));
}

QPoint ChessBoard::point(QChar piece) const
{
    int index = m_boardData.indexOf(piece);

    return QPoint(index / 8 + 1, index % 8 + 1);

}

QHash<QPoint, QChar> ChessBoard::points(QChar piece) const
{
    QHash<QPoint, QChar> points;
    int index;

    QVector<QChar> board(m_boardData);
    while (board.contains(piece))
    {
        index = board.indexOf(piece);
        board[index] = '?';
        qDebug() << "Piece " << piece << " at index " << index;

        int rank = index / 8 + 1;
        int col = index % 8 + 1;

        points[QPoint(col, rank)] = piece;
    }

    return points;
}

/*
 * Sets chess piece on postion at (rank, column)
 * and emits that data change.
 */
void ChessBoard::setData(int column, int rank, QChar value)
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
bool ChessBoard::setDataInternal(int column, int rank, QChar value)
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
    setNrOfMoves(1);
}

/*
 * Helper function that sets the pieces on the board
 * according to the FEN code.
 * Written based on documentation in https://www.chess.com/terms/fen-chess.
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
    for (auto rank = ranks(); rank > 0; --rank)
    {
        for (auto column = 1; column <= columnCount; ++column)
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
    emit boardReset();
}

/*
 * Helper function that gets a FEN code from the current pieces on the board.
 * Written based on documentation in https://www.chess.com/terms/fen-chess.
 */
QString ChessBoard::getFen(QChar player)
{
    QString fen = "";
    int size = m_boardData.size();
    int nrEmptyFields;

    for (auto i=1; i<=8; i++)
    {
        nrEmptyFields = 0;
        int startIndex = size - 8*i;
        int iter = 0;
        while (iter < 8)
        {
            fen = fen;
            int index = startIndex + iter;
            QChar ch = m_boardData.at(index);
            if (ch == ' ') // TODO: CHeck this...
            {
                nrEmptyFields += 1;
            }
            else
            {
                if (nrEmptyFields > 0)
                {
                    fen += QString::number(nrEmptyFields);
                    nrEmptyFields = 0;
                }
                fen += ch;
            }
            iter++;
        }
        if (iter == 8)
        {
            if (nrEmptyFields > 0)
            {
                fen += QString::number(nrEmptyFields);
            }
            fen += "/";
        }
    }
    // Remove last / from fen string.
    fen.chop(1);
    fen += " ";
    // Add current player info.
    fen += player;
    fen += " ";
    // For now keep it simple...
    // Add castling info.
    fen += "KQkq - 0 1";

    /*
     * if Player::whitePlayer:
          if (canCastleShort() && canCastleLong())
          {
            fen += 'KQ';
          }
          else if (canCastleShort() && !canCastleLong())
          {
            fen += 'K'
          }
          else if (!canCastleShort() && canCastleLong())
          {
            fen += 'Q';
          }
          else
          {
            fen += '-';
          }
        }
        else { // Do same for Black Player}
    */
    return fen;
}
