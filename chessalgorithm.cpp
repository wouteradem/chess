#include "chessalgorithm.h"
#include "chessboard.h"
#include <QDebug>
#include <QString>

ChessAlgorithm::ChessAlgorithm(QObject *parent)
    : QObject{parent}
{
    m_board = nullptr;
    m_engine = nullptr;
    m_currentPlayer = NoPlayer;
    m_result = NoResult;

    // Make sure we start with empty moves.
    m_moves.clear();
    m_specialMoves.clear();
    m_engineMoves.clear();
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
    if (result() == value) return;

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
    // Capture edge case when there is somehow now player selected.
    if (currentPlayer() == NoPlayer)
        return false;

    // Check if the current player took the right piece.
    char source = board()->data(colFrom, rankFrom);

    qDebug() << "Current player is : " << currentPlayer();

    if (currentPlayer() == BlackPlayer && source == 'P')
    {
        qDebug() << "[Error] Wrong player. Black player is to move.";
        return false;
    }
    if (currentPlayer() == WhitePlayer && (source == 'p' || source == 'k'))
    {
        qDebug() << "[Error] Wrong player. White player is to move.";
        return false;
    }

    // Move now.
    board()->movePiece(colFrom, rankFrom, colTo, rankTo);

    // Change the player.
    setCurrentPlayer(currentPlayer() == WhitePlayer ? BlackPlayer : WhitePlayer);

    return true;
}

// Entry function from event click!
bool ChessAlgorithm::move(const QPoint &from, const QPoint &to)
{
    QString index;
    char source = board()->data(from.x(), from.y());

    // Check if we are within the board.
    if (!onBoard(to.x(), to.y())) return false;

    // Check if there will be a capture.
    QString whitePieces = "PRNBQK";
    QString blackPieces = "prnbqk";
    char toField = board()->data(to.x(), to.y());

    // Check if we can take or not.
    if (blackPieces.contains(toField) || whitePieces.contains(toField))
    {
        index = toAlgebraic(source, from.x(), from.y(), to.x(), to.y(), true);
    }
    else
    {
        index = toAlgebraic(source, from.x(), from.y(), to.x(), to.y(), false);
    }
    qDebug() << index;

    // Now we try to do the actual move.
    if (m_moves.contains(index))
    {
        return move(from.x(), from.y(), to.x(), to.y());
    }
    qDebug() << "[Error]: Illegal move.";

    return false;
}


void ChessAlgorithm::setEngineMoves(int colFrom, int rankFrom)
{
    char source = board()->data(colFrom, rankFrom);
    qDebug() << source;

    m_engine = new UciEngine();
    m_engine->startEngine("/opt/homebrew/bin/stockfish");
    m_engine->sendCommand("uci");
    m_engine->sendCommand("setoption name Hash value 32");
    m_engine->sendCommand("isready");
    m_engine->sendCommand("ucinewgame");
    m_engine->sendCommand("position fen ");
    m_engine->sendCommand("go depth 3");
}

void ChessAlgorithm::setMoves(int colFrom, int rankFrom)
{
    // Make sure we don't have any old moves.
    if (!m_moves.empty()) m_moves.clear();

    char source = board()->data(colFrom, rankFrom);
    qDebug() << source;
    switch (source)
    {
    case 'P': case 'p':
        setPawnMoves(source, colFrom, rankFrom);
        break;
    case 'N': case 'n':
        setKnightMoves(source, colFrom, rankFrom);
    case 'B': case 'b':
        setBishopMoves(source, colFrom, rankFrom);
    case 'R': case 'r':
         setRookMoves(source, colFrom, rankFrom);
    case 'Q': case 'q':
         setQueenMoves(source, colFrom, rankFrom);
    case 'K': case 'k':
         setKingMoves(source, colFrom, rankFrom);
    }
}

void ChessAlgorithm::setQueenMoves(char piece, int colFrom, int rankFrom)
{
    QString whitePieces = "PRNBQK";
    QString blackPieces = "prnbqk";
    QString move = "";
    int colTo, rankTo;

    for (int ranks=-1; ranks<2; ranks++)
    {
         for(int cols=-1; cols<2; cols++)
         {
             for(size_t i=1; i<8; i++)
             {

                 colTo = colFrom + (cols * i);
                 rankTo = rankFrom + (ranks * i);

                 // Check if on board.
                 if (onBoard(colTo, rankTo))
                 {
                     // Check what is on that field.
                     char toField = board()->data(colTo, rankTo);

                     // Field must be empty and NOT contain a white piece.
                     if (piece == 'Q')
                     {
                         if (whitePieces.contains(toField))
                         {
                             qDebug() << "[Log] Illegal move. Bumped on white piece.";
                             break;
                         }
                         if (toField == ' ')
                         {
                             toField = board()->data(colTo, rankTo);
                             if (blackPieces.contains(toField))
                             {
                                 move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                 qDebug() << "[Log] Legal move. Capture of black piece.";
                             }
                             else
                             {
                                 move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, false);
                                 qDebug() << "[Log] Legal move. To empty place.";

                             }
                             m_moves[move] = false;
                         }
                         else
                         {
                             qDebug() << "[Log] Illegal position for Queen.";
                         }
                     }
                     if (piece == 'q')
                     {
                         if (blackPieces.contains(toField))
                         {
                             qDebug() << "[Log] Illegal move. Bumped on white piece.";
                             break;
                         }
                         if (toField == ' ')
                         {
                             toField = board()->data(colTo, rankTo);
                             if (whitePieces.contains(toField))
                             {
                                 move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                 qDebug() << "[Log] Legal move. Capture of black piece.";
                             }
                             else
                             {
                                 move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, false);
                                 qDebug() << "[Log] Legal move. To empty place.";

                             }
                             m_moves[move] = false;
                         }
                         else
                         {
                             qDebug() << "[Log] Illegal position for Queen.";
                         }
                     }
                 }
             }
         }
    }
}

void ChessAlgorithm::setKingMoves(char piece, int colFrom, int rankFrom)
{

    QString whitePieces = "PRNBQK";
    QString blackPieces = "prnbqk";
    QString move = "";
    int colTo, rankTo;

    for (int ranks=-1; ranks<2; ranks++)
    {
         for(int cols=-1; cols<2; cols++)
         {
             for(size_t i=1; i<2; i++)
             {

                 colTo = colFrom + (cols * i);
                 rankTo = rankFrom + (ranks * i);

                 // Check if on board.
                 if (onBoard(colTo, rankTo))
                 {
                     // Check what is on that field.
                     char toField = board()->data(colTo, rankTo);

                     // Field must be empty and NOT contain a white piece.
                     if (piece == 'K')
                     {
                         if (whitePieces.contains(toField))
                         {
                             qDebug() << "[Log] Illegal move. Bumped on white piece.";
                             break;
                         }
                         if (toField == ' ')
                         {
                             toField = board()->data(colTo, rankTo);
                             if (blackPieces.contains(toField))
                             {
                                 move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                 qDebug() << "[Log] Legal move. Capture of black piece.";
                             }
                             else
                             {
                                 move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, false);
                                 qDebug() << "[Log] Legal move. To empty place.";

                             }
                             m_moves[move] = false;
                         }
                         else
                         {
                             qDebug() << "[Log] Illegal position for Queen.";
                         }
                     }
                     if (piece == 'k')
                     {
                         if (blackPieces.contains(toField))
                         {
                             qDebug() << "[Log] Illegal move. Bumped on white piece.";
                             break;
                         }
                         if (toField == ' ')
                         {
                             toField = board()->data(colTo, rankTo);
                             if (whitePieces.contains(toField))
                             {
                                 move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                 qDebug() << "[Log] Legal move. Capture of black piece.";
                             }
                             else
                             {
                                 move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, false);
                                 qDebug() << "[Log] Legal move. To empty place.";

                             }
                             m_moves[move] = false;
                         }
                         else
                         {
                             qDebug() << "[Log] Illegal position for Queen.";
                         }
                     }
                 }
             }
         }
    }

    // Check for long-castling.
    for (int cols=-3; cols<0; cols++)
    {
         colTo = colFrom + cols;
         rankTo = rankFrom;

         if (onBoard(colTo, rankTo))
         {
             // Field must be empty and NOT contain a white piece.
             if (piece == 'K')
             {
                 // Check what is on that field.
                 char toField = board()->data(colTo, rankTo);
                 if (toField == ' ')
                 {
                     colTo = colFrom - 2;
                     rankTo = rankFrom;
                     move = toAlgebraic('K', colFrom, rankFrom, colTo, rankTo, false);
                     qDebug() << "[Log] Legal move. Long castle.";
                     m_moves[move] = false;
                 }
                 else
                 {
                     qDebug() << "Field not empty.";
                 }
             }
         }
    }

    // Check for short-castling.
    for (int cols=1; cols<3; cols++)
    {
         colTo = colFrom + cols;
         rankTo = rankFrom;

         if (onBoard(colTo, rankTo))
         {
             if (piece == 'K')
             {
                 // Check what is on that field.
                 char toField = board()->data(colTo, rankTo);
                 if (toField == ' ')
                 {
                     colTo = colFrom + 2;
                     rankTo = rankFrom;
                     move = toAlgebraic('K', colFrom, rankFrom, colTo, rankTo, false);
                     // TODO: Forced rook move.
                     if (this->move(8, 1, 6, 1)) // TODO: DOES NOT WORK PROPERLY!
                     {
                         qDebug() << "[Log] Legal move. Short castle.";
                     }
                     m_moves[move] = false;
                 }
                 else
                 {
                     qDebug() << "Field not empty.";
                 }
             }
         }
    }

}

void ChessAlgorithm::setKnightMoves(char piece, int colFrom, int rankFrom)
{

    QString whitePieces = "PRNBQK";
    QString blackPieces = "prnbqk";
    QString move = "";

    if (piece == 'N' || piece == 'n')
    {
        int colTo, rankTo, x, y;

        // Knight has 8 possible moves.
        for (size_t i=0; i<8; i++)
        {
            switch (i)
            {
            case 0:
                x = 1;
                y = 2;
                break;
            case 1:
                x = 1;
                y = -2;
                break;

            case 2:
                x = -1;
                y = 2;
                break;

            case 3:
                x = -1;
                y = -2;
                break;

            case 4:
                x = 2;
                y = 1;
                break;

            case 5:
                x = 2;
                y = -1;
                break;

            case 6:
                x = -2;
                y = 1;
                break;

            case 7:
                x = -2;
                y = -1;
                break;
            }
            colTo = colFrom + x;
            rankTo = rankFrom + y;

            // Check if on board.
            if (onBoard(colTo, rankTo))
            {
                // Check what is on that field.
                char toField = board()->data(colTo, rankTo);
                // Field must be empty and NOT contain a white piece.
                if (piece == 'N')
                {
                    if (toField == ' ' || !whitePieces.contains(toField))
                    {
                        toField = board()->data(colTo, rankTo);
                        if (blackPieces.contains(toField))
                        {
                            move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                            qDebug() << "[Log] Legal move. Capture of black piece.";
                        }
                        else
                        {
                            move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, false);
                            qDebug() << "[Log] Legal move. To empty place.";

                        }
                        m_moves[move] = false;
                    }
                }
                else if (piece == 'n')
                {
                    if (toField == ' ' || !blackPieces.contains(toField))
                    {
                        if (whitePieces.contains(toField))
                        {
                            move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                            qDebug() << "[Log] Legal move. Capture of white piece.";
                        }
                        else
                        {
                            move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, false);
                            qDebug() << "[Log] Legal move. To empty place.";

                        }
                        m_moves[move] = false;
                    }
                }
            }
            else
            {
                qDebug() << "[Error] Illegal move. Off board.";
            }
        }
    }
}

void ChessAlgorithm::setBishopMoves(char piece, int colFrom, int rankFrom)
{

    QString whitePieces = "PRNBQK";
    QString blackPieces = "prnbqk";
    QString move = "";
    int colTo, rankTo;

    for (int ranks=-1; ranks<2; ranks++)
    {
        for(int cols=-1; cols<2; cols++)
        {
            for(size_t i=1; i<8; i++)
            {

                colTo = colFrom + (cols * i);
                rankTo = rankFrom + (ranks * i);

                // Make sure the bishop is moving diagonally.
                bool sameCol = colTo == colFrom;
                bool sameRank = rankTo == rankFrom;

                // Check if on board.
                if (onBoard(colTo, rankTo))
                {
                    // Check what is on that field.
                    char toField = board()->data(colTo, rankTo);

                    // Field must be empty and NOT contain a white piece.
                    if (piece == 'B')
                    {
                        if (sameCol)
                        {
                            qDebug() << "[Log] Illegal move. Same column.";
                            break;
                        }
                        if (sameRank)
                        {
                            qDebug() << "[Log] Illegal move. Same rank.";
                            break;
                        }
                        if (whitePieces.contains(toField))
                        {
                            qDebug() << "[Log] Illegal move. Bumped on white piece.";
                            break;
                        }
                        if (toField == ' ')
                        {
                            toField = board()->data(colTo, rankTo);
                            if (blackPieces.contains(toField))
                            {
                                move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                qDebug() << "[Log] Legal move. Capture of black piece.";
                            }
                            else
                            {
                                move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, false);
                                qDebug() << "[Log] Legal move. To empty place.";

                            }
                            m_moves[move] = false;
                        }
                        else
                        {
                            qDebug() << "[Log] Illegal position for Bisshop.";
                        }
                    }
                    else if (piece == 'b')
                    {
                        if (sameCol)
                        {
                            qDebug() << "[Log] Illegal move. Same column.";
                            break;
                        }
                        if (sameRank)
                        {
                            qDebug() << "[Log] Illegal move. Same rank.";
                            break;
                        }
                        if (blackPieces.contains(toField))
                        {
                            qDebug() << "[Log] Illegal move. Bumped on piece of same colour.";
                            break;
                        }
                        if (toField == ' ')
                        {
                            if (whitePieces.contains(toField))
                            {
                                move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                qDebug() << "[Log] Legal move. Capture of white piece.";
                            }
                            else
                            {
                                move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, false);
                                qDebug() << "[Log] Legal move. To empty place.";

                            }
                            m_moves[move] = false;
                        }
                    }
                }
                else
                {
                    qDebug() << "[Error] Illegal move. Off board.";
                }
            }
        }
    }
}

void ChessAlgorithm::setRookMoves(char piece, int colFrom, int rankFrom)
{
    qDebug() << "Setting rook moves.";

    QString whitePieces = "PRNBQK";
    QString blackPieces = "prnbqk";
    QString move = "";
    int colTo, rankTo;

    for (int rank=-1; rank<2; rank++)
    {
        for(int col=-1; col<2; col++)
        {
            if (rank != col && rank != (-col))
            {
                for (size_t i=1; i<8; i++)
                {
                    colTo = colFrom + (col * i);
                    rankTo = rankFrom + (rank * i);

                    // Check if on board.
                    if (onBoard(colTo, rankTo))
                    {
                        // Check what is on that field.
                        char toField = board()->data(colTo, rankTo);

                        // Field must be empty and NOT contain a white piece.
                        if (piece == 'R')
                        {
                            if (toField == ' ' || !whitePieces.contains(toField))
                            {
                                toField = board()->data(colTo, rankTo);
                                if (blackPieces.contains(toField))
                                {
                                    move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                    qDebug() << "[Log] Legal move. Capture of black piece.";
                                }
                                else
                                {
                                    move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, false);
                                    qDebug() << "[Log] Legal move. To empty place.";

                                }
                                m_moves[move] = false;
                            }
                            else
                            {
                                qDebug() << "[Log] Illegal position for Bisshop.";
                            }
                        }
                        else if (piece == 'r')
                        {
                            if (toField == ' ' || !blackPieces.contains(toField))
                            {
                                if (whitePieces.contains(toField))
                                {
                                    move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                    qDebug() << "[Log] Legal move. Capture of white piece.";
                                }
                                else
                                {
                                    move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, false);
                                    qDebug() << "[Log] Legal move. To empty place.";

                                }
                                m_moves[move] = false;
                            }
                        }
                    }
                    else
                    {
                        qDebug() << "[Error] Illegal move. Off board.";
                    }
                }
            }
        }
    }
}

void ChessAlgorithm::setPawnMoves(char piece, int colFrom, int rankFrom)
{
    QString move = "";
    if (piece == 'P')
    {
        if (rankFrom == 2)
        {
            move = toAlgebraic(piece, colFrom, rankFrom, colFrom, rankFrom + 2, false);
            m_moves[move] = false;
        }
        move = toAlgebraic(piece, colFrom, rankFrom, colFrom, rankFrom + 1, false);
        m_moves[move] = false;

        // Add take moves.
        char leftTake = board()->data(colFrom - 1, rankFrom + 1);
        char rightTake = board()->data(colFrom + 1, rankFrom + 1);
        QString whitePieces = "PRNBQK";
        if (leftTake != ' ' && !whitePieces.contains(leftTake))
        {
            move = toAlgebraic(piece, colFrom, rankFrom, colFrom - 1, rankFrom + 1, true);
            m_moves[move] = false;
        }
        else if (rightTake != ' ' && !whitePieces.contains(rightTake))
        {
            move = toAlgebraic(piece, colFrom, rankFrom, colFrom + 1, rankFrom + 1, true);
            m_moves[move] = false;
        }
        // En passant.
        if (rankFrom == 5)
        {
            leftTake = board()->data(colFrom - 1, rankFrom);
            rightTake = board()->data(colFrom + 1, rankFrom);
            if (leftTake != ' ' && !whitePieces.contains(leftTake))
            {
                move = toAlgebraic(piece, colFrom, rankFrom, colFrom - 1, rankFrom + 1, true);
                m_moves[move] = false;
            }
            else if (rightTake != ' ' && !whitePieces.contains(rightTake))
            {
                move = toAlgebraic(piece, colFrom, rankFrom, colFrom + 1, rankFrom + 1, true);
                m_moves[move] = false;
            }
            // TODO: We need to remove the black piece.
        }
    }
    else if (piece == 'p')
    {
        move = toAlgebraic(piece, colFrom, rankFrom, colFrom, rankFrom - 1, false);
        m_moves[move] = false;
        if (rankFrom == 7)
        {
            QString move = toAlgebraic(piece, colFrom, rankFrom, colFrom, rankFrom - 2, false);
            m_moves[move] = false;
        }

        // Add take moves.
        // TODO: Add en-passent capture.
        char leftTake = board()->data(colFrom + 1, rankFrom - 1);
        char rightTake = board()->data(colFrom - 1, rankFrom - 1);
        QString blackPieces = "prnbqk";
        if (leftTake != ' ' && !blackPieces.contains(leftTake))
        {
            QString move = toAlgebraic(piece, colFrom, rankFrom, colFrom + 1, rankFrom - 1, true);
            m_moves[move] = false;
        }
        else if (rightTake != ' ' && !blackPieces.contains(rightTake))
        {
            QString move = toAlgebraic(piece, colFrom, rankFrom, colFrom - 1, rankFrom - 1, true);
            m_moves[move] = false;
        }
    }
}

// Converts a chess move to algebraic notation.
// https://www.chess.com/terms/chess-notation.
QString ChessAlgorithm::toAlgebraic(char piece, int colFrom, int rankFrom, int colTo, int rankTo, bool canTake)
{
    // TODO: Can be written shorter!

    QString algNot = "";

    QChar colToChar = QChar('a' + colTo - 1);
    QChar colFromChar = QChar('a' + colFrom - 1);

    // Pawns don't have the piece.
    if (piece == 'P' || piece == 'p')
    {
        // Case capture a piece and normal pawn move.
        algNot = canTake ? colFromChar + 'x' + colToChar + QString::number(rankTo) : colToChar + QString::number(rankTo);
    }
    else if (piece == 'N' || piece == 'n')
    {
        // Case capture a piece and normal Knight move.
        algNot = "Nx";
        algNot = canTake ? algNot + colToChar + QString::number(rankTo) : 'N' + colToChar + QString::number(rankTo);
    }
    else if (piece == 'B' || piece == 'b')
    {
        algNot = "Bx";
        algNot = canTake ? algNot + colToChar + QString::number(rankTo) : 'B' + colToChar + QString::number(rankTo);
    }
    else if (piece == 'R' || piece == 'r')
    {
        algNot = "Rx";
        algNot = canTake ? algNot + colToChar + QString::number(rankTo) : 'R' + colToChar + QString::number(rankTo);
    }
    else if (piece == 'Q' || piece == 'q')
    {
        algNot = "Qx";
        algNot = canTake ? algNot + colToChar + QString::number(rankTo) : 'Q' + colToChar + QString::number(rankTo);
    }
    else if (piece == 'K' || piece == 'k')
    {
        algNot = "Kx";
        algNot = canTake ? algNot + colToChar + QString::number(rankTo) : 'Q' + colToChar + QString::number(rankTo);
    }

    qDebug() << algNot;

    return algNot;
}

QPoint ChessAlgorithm::toCoordinates(QString move)
{

    if (move.contains("x"))
    {
        move.replace("x","");
    }
    if (move.length() == 3)
    {
        move.remove(0, 1);
    }

    QChar col = move.at(0);
    int colTo = col.toLatin1() - 'a' + 1;

    QChar rank = move.at(1);
    int rankTo = rank.toLatin1() - '0';

    return QPoint(colTo, rankTo);
}

bool ChessAlgorithm::onBoard(int colTo, int rankTo)
{
    bool onBoard = false;
    if (colTo < 1 || colTo > board()->columns())
    {
        return onBoard;
    }
    if (rankTo < 1 || rankTo > board()->ranks())
    {
        return onBoard;
    }

    return true;
}
