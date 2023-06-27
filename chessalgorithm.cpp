#include "chessalgorithm.h"
#include "chessboard.h"
#include <QDebug>
#include <QString>
#include <QChar>

ChessAlgorithm::ChessAlgorithm(QObject *parent)
    : QObject{parent}
{
    m_board = nullptr;
    m_engine = new UciEngine();

    m_currentPlayer = NoPlayer;
    m_result = NoResult;

    // En-passent.
    m_enpassent_black = false;
    m_enpassent_white = false;

    // Castling.
    m_whitecastled = false;
    m_blackcastled = false;
    m_shortcastle_white = false;
    m_longcastle_white = false;
    m_shortcastle_black = false;
    m_longcastle_black = false;
    m_whiteCannotCastle = false;
    m_blackCannotCastle = false;

    // Castling condition.
    m_whiteRookMoved = false;
    m_blackRookMoved = false;
    m_whiteKingMoved = false;
    m_blackKingMoves = false;

    // Make sure we start with empty moves.
    m_moves.clear();
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
    if (m_board)
    {
        delete m_board;
        delete m_engine;
    }
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
        emit gameOver(m_result);
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

void ChessAlgorithm::setCurrentMove(QString move)
{
    m_currentMove = move;
}

QString ChessAlgorithm::getFENBoard()
{
    // Get Engine move.
    QString fenMove;
    if (currentPlayer() == Player::WhitePlayer)
        fenMove = board()->getFen('w');
    else if (currentPlayer() == Player::BlackPlayer)
        fenMove = board()->getFen('b');

    return fenMove;
}

bool ChessAlgorithm::move(int colFrom, int rankFrom, int colTo, int rankTo)
{
    qDebug() << "Current player is : " << currentPlayer();

    // Capture edge case when there is somehow now player selected.
    if (currentPlayer() == NoPlayer)
        return false;

    // Get the current piece.
    QChar source = board()->data(colFrom, rankFrom);
    QString whitePieces = "PRNBQK";
    QString blackPieces = "prnbqk";
    if (currentPlayer() == BlackPlayer && !blackPieces.contains(source))
    {
        qDebug() << "[Error] Wrong player. Black player is to move.";
        return false;
    }
    if (currentPlayer() == WhitePlayer && !whitePieces.contains(source))
    {
        qDebug() << "[Error] Wrong player. White player is to move.";
        return false;
    }

    if (m_moves.count() == 0)
    {
        qDebug() << "Stalemate! Draw!";
    }

    // CAUTION: Here we are doing something non-intuitive!
    // We need to do the provisional move here.
    // Whitout emitting something so we don't update the UI.
    QChar pieceOnToSource = board()->data(colTo, colFrom);
    board()->setDataInternal(colTo, rankTo, board()->data(colFrom, rankFrom));
    board()->setDataInternal(colFrom, rankFrom, ' ');

    // If current player is white(black) and wants to make a move
    // check if black(white) doesn't give a check. So change current player.
    bool switchplayer = true;
    if (check(switchplayer))
    {
        // Revert the move.
        board()->setDataInternal(colFrom, rankFrom, board()->data(colTo, rankTo));
        board()->setDataInternal(colTo, rankTo, pieceOnToSource);

        emit checkYourself();

        return false;
    }

    // Revert the move.
    board()->setDataInternal(colFrom, rankFrom, board()->data(colTo, rankTo));
    board()->setDataInternal(colTo, rankTo, pieceOnToSource);

    // Now do the REAL move.
    board()->movePiece(colFrom, rankFrom, colTo, rankTo);

    // Check if the new move gave a check.
    switchplayer = false;
    if (check(switchplayer))
    {
        // Check if we aren't checkmated.
        bool checkMate = this->checkMate();
        if (checkMate && currentPlayer() == ChessAlgorithm::WhitePlayer)
        {
            emit gameOver(ChessAlgorithm::WhiteWin);
        }
        else if (checkMate && currentPlayer() == ChessAlgorithm::BlackPlayer)
        {
            emit gameOver(ChessAlgorithm::BlackWin);
        }

        if (currentPlayer() == BlackPlayer)
        {
            board()->setWhiteChecked(true);
            emit checked(QPoint(5, 8));
        }
        else if (currentPlayer() == WhitePlayer)
        {
            board()->setBlackChecked(true);
            emit checked(QPoint(5, 8));
        }
    }
    else
    {
        emit unChecked();
    }

    // Check for en-passent.
    if (currentPlayer() == WhitePlayer && m_enpassent_white)
    {
        board()->setDataInternal(colTo, rankTo - 1, ' ');
        m_enpassent_white = false;
    }
    else if (currentPlayer() == BlackPlayer && m_enpassent_black)
    {
        board()->setDataInternal(colTo, rankTo + 1, ' ');
        m_enpassent_black = false;
    }

    // White castle.
    if (m_shortcastle_white && !m_whiteCannotCastle && colTo == 7 && rankTo == 1)
    {
        board()->setDataInternal(8, 1, ' ');
        board()->setDataInternal(6, 1, 'R');
        m_whitecastled = true;
    }
    if (m_longcastle_white && !m_whiteCannotCastle && colTo == 3 && rankTo == 1)
    {
        board()->setDataInternal(1, 1, ' ');
        board()->setDataInternal(4, 1, 'R');
        m_whitecastled = true;
    }

    // Black castle.
    if (m_shortcastle_black && !m_blackCannotCastle && colTo == 7 && rankTo == 8)
    {
        board()->setDataInternal(8, 8, ' ');
        board()->setDataInternal(6, 8, 'r');
        m_blackcastled = true;
    }
    if (m_longcastle_black && !m_blackCannotCastle && colTo == 3 && rankTo == 8)
    {
        board()->setDataInternal(1, 8, ' ');
        board()->setDataInternal(4, 8, 'r');
        m_blackcastled = true;
    }

    // Finally change the player.
    setCurrentPlayer(currentPlayer() == WhitePlayer ? BlackPlayer : WhitePlayer);

    return true;
}

// Entry function from event click!
bool ChessAlgorithm::move(const QPoint &from, const QPoint &to)
{
    QString index;
    QChar source = board()->data(from.x(), from.y());

    // Validate if we can actually castle.
    if (source == 'K' && to.x() != 7 && to.y() != 1)
    {
        m_whiteCannotCastle = true;
    }
    if (source == 'K' && to.x() != 3 && to.y() != 1)
    {
        m_whiteCannotCastle = true;
    }
    if (source == 'k' && to.x() != 7 && to.y() != 8)
    {
        m_blackCannotCastle = true;
    }
    if (source == 'k' && to.x() != 3 && to.y() != 8)
    {
        m_blackCannotCastle = true;
    }
    if (source == 'R' && to.x() != 6)
    {
        m_whiteCannotCastle = true;
    }
    if (source == 'R' && to.x() != 4)
    {
        m_whiteCannotCastle = true;
    }
    if (source == 'r' && to.x() != 6)
    {
        m_blackCannotCastle = true;
    }
    if (source == 'r' && to.x() != 4)
    {
        m_blackCannotCastle = true;
    }

    // Check if we are within the board.
    if (!onBoard(to.x(), to.y())) return false;

    // Check if there will be a capture.
    QString whitePieces = "PRNBQK";
    QString blackPieces = "prnbqk";

    QChar toField = board()->data(to.x(), to.y());

    // Check if we can take or not.
    if (currentPlayer() == WhitePlayer)
    {
        if (blackPieces.contains(toField) || m_enpassent_white)
        {
            index = toAlgebraic(source, from.x(), from.y(), to.x(), to.y(), true);
        }
        else if (source == 'K' && from.x() == 5 && to.x() == 7)
        {
            index = toAlgebraicCastle(source, from.x(), from.y(), to.x(), to.y(), true);
        }
        else
        {
            index = toAlgebraic(source, from.x(), from.y(), to.x(), to.y(), false);
        }
    }
    else if (m_currentPlayer == BlackPlayer)
    {
        if (whitePieces.contains(toField) || m_enpassent_black)
        {
            index = toAlgebraic(source, from.x(), from.y(), to.x(), to.y(), true);
        }
        else if (source == 'k' && from.x() == 5 && to.x() == 7)
        {
            index = toAlgebraicCastle(source, from.x(), from.y(), to.x(), to.y(), true);
        }
        else
        {
            index = toAlgebraic(source, from.x(), from.y(), to.x(), to.y(), false);
        }
    }
    // Now we try to do the actual move.
    if (m_moves.contains(index))
    {
        setCurrentMove(index);

        return move(from.x(), from.y(), to.x(), to.y());
    }

    return false;
}

void ChessAlgorithm::setEngineMoves(QString fen)
{
    m_engine->startEngine("/opt/homebrew/bin/stockfish");
    m_engine->sendCommand("uci");
    m_engine->sendCommand("setoption name Hash value 32");
    m_engine->sendCommand("isready");
    m_engine->sendCommand("ucinewgame");
    m_engine->sendCommand("position fen " + fen);
    m_engine->sendCommand("go depth 3");
}

void ChessAlgorithm::setMoves(int colFrom, int rankFrom)
{
    // Make sure we don't have any old moves.
    if (!m_moves.empty()) m_moves.clear();

    QChar source = board()->data(colFrom, rankFrom);
    switch (source.toLatin1())
    {
    case 'P': case 'p':
        setPawnMoves(source, colFrom, rankFrom);
        break;
    case 'N': case 'n':
        setKnightMoves(source, colFrom, rankFrom);
        break;
    case 'B': case 'b':
        setBishopMoves(source, colFrom, rankFrom);
        break;
    case 'R': case 'r':
        setRookMoves(source, colFrom, rankFrom);
        break;
    case 'Q': case 'q':
        setQueenMoves(source, colFrom, rankFrom);
        break;
    case 'K': case 'k':
        setKingMoves(source, colFrom, rankFrom);
        break;
    }
}

void ChessAlgorithm::setQueenMoves(QChar piece, int colFrom, int rankFrom)
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
                     QChar toField = board()->data(colTo, rankTo);

                     // Field must be empty and NOT contain a white piece.
                     if (piece == 'Q')
                     {
                         if (whitePieces.contains(toField))
                         {
                             qDebug() << "[Log] Illegal move. Bumped on white piece.";
                             break;
                         }
                         if (toField == ' ' || !whitePieces.contains(toField))
                         {
                             toField = board()->data(colTo, rankTo);
                             if (blackPieces.contains(toField))
                             {
                                 move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                 qDebug() << "[Log] Legal move. Capture of black piece.";
                                 m_moves[move] = false;
                                 break;
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
                             break;
                         }
                     }
                     if (piece == 'q')
                     {
                         if (blackPieces.contains(toField))
                         {
                             qDebug() << "[Log] Illegal move. Bumped on white piece.";
                             break;
                         }
                         if (toField == ' ' || !blackPieces.contains(toField))
                         {
                             toField = board()->data(colTo, rankTo);
                             if (whitePieces.contains(toField))
                             {
                                 move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                 qDebug() << "[Log] Legal move. Capture of black piece.";
                                 m_moves[move] = false;
                                 break;
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
                             break;
                         }
                     }
                 }
             }
         }
    }
}

void ChessAlgorithm::setKingMoves(QChar piece, int colFrom, int rankFrom)
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
                     QChar toField = board()->data(colTo, rankTo);

                     // Field must be empty and NOT contain a white piece.
                     if (piece == 'K')
                     {
                         if (whitePieces.contains(toField))
                         {
                             qDebug() << "[Log] Illegal move. Bumped on white piece.";
                             break;
                         }
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

    for (int cols=1; cols<3; cols++)
    {
        colTo = colFrom + cols;
        rankTo = rankFrom;

        if (onBoard(colTo, rankTo))
        {
            if (piece == 'K' || piece == 'k')
            {
                QChar toField = board()->data(colTo, rankTo);
                if (toField == ' ')
                {
                    if (piece == 'K' && !m_whiteCannotCastle)
                    {
                        move = toAlgebraicCastle('K', colFrom, rankFrom, colTo, rankTo, true);
                    }
                    else if (piece == 'k' && !m_blackCannotCastle)
                    {
                        move = toAlgebraicCastle('k', colFrom, rankFrom, colTo, rankTo, true);
                    }

                    // Check short castle colour.
                    if (move.contains("0-0"))
                    {
                       m_shortcastle_white = true;
                    }
                    if (move.contains("0-0"))
                    {
                       m_shortcastle_black = true;
                    }
                    m_moves[move] = false;
                }
            }
        }
    }

    // Long castle.
    for (int cols=-2; cols<0; cols++)
    {
        colTo = colFrom + cols;
        rankTo = rankFrom;
        if (onBoard(colTo, rankTo))
        {
            // Field must be empty and NOT contain a white piece.
            if (piece == 'K' || piece == 'k')
            {
                QChar toField = board()->data(colTo, rankTo);
                if (toField == ' ')
                {
                    colTo = colFrom - 2;
                    rankTo = rankFrom;
                    if (piece == 'K' && !m_whiteCannotCastle)
                    {
                       move = toAlgebraic('K', colFrom, rankFrom, colTo, rankTo, false);
                    }
                    else if (piece == 'k' && !m_blackCannotCastle)
                    {
                       move = toAlgebraic('k', colFrom, rankFrom, colTo, rankTo, false);
                    }

                    // Check long castle colour.
                    if (move.contains("Kc1"))
                    {
                       m_longcastle_white = true;
                    }
                    if (move.contains("Kc8"))
                    {
                       m_longcastle_black = true;
                    }
                    m_moves[move] = false;
                }
            }
        }
    }
}

void ChessAlgorithm::setKnightMoves(QChar piece, int colFrom, int rankFrom)
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
                QChar toField = board()->data(colTo, rankTo);
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

void ChessAlgorithm::setBishopMoves(QChar piece, int colFrom, int rankFrom)
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
                    QChar toField = board()->data(colTo, rankTo);

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
                        if (toField == ' ' || !whitePieces.contains(toField))
                        {
                            toField = board()->data(colTo, rankTo);
                            if (blackPieces.contains(toField))
                            {
                                move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                qDebug() << "[Log] Legal move. Capture of black piece.";
                                m_moves[move] = false;
                                break;
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
                        if (toField == ' ' || !blackPieces.contains(toField))
                        {
                            if (whitePieces.contains(toField))
                            {
                                move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                qDebug() << "[Log] Legal move. Capture of white piece.";
                                m_moves[move] = false;
                                break;
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
                            break;
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

void ChessAlgorithm::setRookMoves(QChar piece, int colFrom, int rankFrom)
{
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
                        QChar toField = board()->data(colTo, rankTo);

                        // Field must be empty and NOT contain a white piece.
                        if (piece == 'R')
                        {
                            if (whitePieces.contains(toField))
                            {
                                qDebug() << "[Log] Illegal move. Bumped on white piece.";
                                break;
                            }
                            if (toField == ' ' || !whitePieces.contains(toField))
                            {
                                toField = board()->data(colTo, rankTo);
                                if (blackPieces.contains(toField))
                                {
                                    move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                    qDebug() << "[Log] Legal move. Capture of black piece.";
                                    m_moves[move] = false;
                                    break;
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
                                qDebug() << "[Log] Illegal position for rook.";
                            }
                        }
                        else if (piece == 'r')
                        {
                            if (toField == ' ' || !blackPieces.contains(toField))
                            {
                                toField = board()->data(colTo, rankTo);
                                if (whitePieces.contains(toField))
                                {
                                    move = toAlgebraic(piece, colFrom, rankFrom, colTo, rankTo, true);
                                    qDebug() << "[Log] Legal move. Capture of white piece.";
                                    m_moves[move] = false;
                                    break;
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
                                qDebug() << "[Log] Illegal position for rook.";
                                break;
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

void ChessAlgorithm::setPawnMoves(QChar piece, int colFrom, int rankFrom)
{
    QString move = "";
    if (piece == 'P')
    {
        if (rankFrom == 2)
        {
            if (board()->data(colFrom, rankFrom + 2) == ' ')
            {
                move = toAlgebraic(piece, colFrom, rankFrom, colFrom, rankFrom + 2, false);
                m_moves[move] = false;
            }
        }
        if (board()->data(colFrom, rankFrom + 1) == ' ')
        {
            move = toAlgebraic(piece, colFrom, rankFrom, colFrom, rankFrom + 1, false);
            m_moves[move] = false;
        }
        // Add take moves.
        bool leftOnBoard = this->onBoard(colFrom - 1, rankFrom + 1);
        QChar leftTake = board()->data(colFrom - 1, rankFrom + 1);

        bool rightOnBoard = this->onBoard(colFrom + 1, rankFrom + 1);
        QChar rightTake = board()->data(colFrom + 1, rankFrom + 1);

        QString whitePieces = "PRNBQK";
        if (leftOnBoard && leftTake != ' ' && !whitePieces.contains(leftTake))
        {
            move = toAlgebraic(piece, colFrom, rankFrom, colFrom - 1, rankFrom + 1, true);
            m_moves[move] = false;
        }
        else if (rightOnBoard && rightTake != ' ' && !whitePieces.contains(rightTake))
        {
            move = toAlgebraic(piece, colFrom, rankFrom, colFrom + 1, rankFrom + 1, true);
            m_moves[move] = false;
        }
        // En passant.
        leftTake = board()->data(colFrom - 1, rankFrom);
        rightTake = board()->data(colFrom + 1, rankFrom);
        if (rankFrom == 5)
        {
            if (leftTake != ' ' && !whitePieces.contains(leftTake))
            {
                move = toAlgebraic(piece, colFrom, rankFrom, colFrom - 1, rankFrom + 1, true);
                m_moves[move] = false;
                m_enpassent_white = true;
            }
            else if (rightTake != ' ' && !whitePieces.contains(rightTake))
            {
                move = toAlgebraic(piece, colFrom, rankFrom, colFrom + 1, rankFrom + 1, true);
                m_moves[move] = false;
                m_enpassent_white = true;
            }
        }
    }
    else if (piece == 'p')
    {
        if (board()->data(colFrom, rankFrom - 1) == ' ') {
            move = toAlgebraic(piece, colFrom, rankFrom, colFrom, rankFrom - 1, false);
            m_moves[move] = false;
        }
        if (rankFrom == 7)
        {
            if (board()->data(colFrom, rankFrom - 2) == ' ')
            {
                QString move = toAlgebraic(piece, colFrom, rankFrom, colFrom, rankFrom - 2, false);
                m_moves[move] = false;
            }
        }

        // Add take moves.
        QChar leftTake = board()->data(colFrom + 1, rankFrom - 1);
        QChar rightTake = board()->data(colFrom - 1, rankFrom - 1);
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

        // En passant.
        leftTake = board()->data(colFrom - 1, rankFrom);
        rightTake = board()->data(colFrom + 1, rankFrom);
        if (rankFrom == 4)
        {
            if (leftTake != ' ' && !blackPieces.contains(leftTake))
            {
                move = toAlgebraic(piece, colFrom, rankFrom, colFrom - 1, rankFrom - 1, true);
                m_enpassent_black = true;
                m_moves[move] = false;
            }
            else if (rightTake != ' ' && !blackPieces.contains(rightTake))
            {
                move = toAlgebraic(piece, colFrom, rankFrom, colFrom + 1, rankFrom - 1, true);
                m_enpassent_black = true;
                m_moves[move] = false;
            }
        }
    }
}

// Converts a chess move to algebraic notation.
// https://www.chess.com/terms/chess-notation.
QString ChessAlgorithm::toAlgebraic(QChar piece, int colFrom, int rankFrom, int colTo, int rankTo, bool canTake)
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
        algNot = canTake ? algNot + colToChar + QString::number(rankTo) : 'K' + colToChar + QString::number(rankTo);
    }

    qDebug() << algNot;

    return algNot;
}


QString ChessAlgorithm::toAlgebraicCastle(QChar piece, int colFrom, int rankFrom, int colTo, int rankTo, bool canCastleShort)
{
    QString algNot = "";

        if (!canCastleShort)
        {
            algNot = "0-0-0";
        }
        else
        {
            algNot = "0-0";
        }

    qDebug() << algNot;

    return algNot;
}

QPoint ChessAlgorithm::toCoordinates(QString move)
{
    if (move.contains("0-0"))
    {
        if (currentPlayer() == BlackPlayer)
        {
            return QPoint(7, 8);
        }
        else if (currentPlayer() == WhitePlayer)
        {
            return QPoint(7, 1);
        }
    }
    if (move.contains("0-0-0"))
    {
        if (currentPlayer() == BlackPlayer)
        {
            return QPoint(3, 8);
        }
        else if (currentPlayer() == WhitePlayer)
        {
            return QPoint(3, 1);
        }
    }
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

bool ChessAlgorithm::check(bool switchplayer)
{   
    // Store the current values.
    bool enpassentWhite = m_enpassent_white;
    bool enpassentBlack = m_enpassent_black;
    bool shortcastleWhite = m_shortcastle_white;
    bool longcastleWhite = m_longcastle_white;

    bool shortcastle_white = m_shortcastle_white;
    bool longcastle_white = m_longcastle_white;
    bool shortcastle_black = m_shortcastle_black;
    bool longcastle_black = m_longcastle_black;

    QString pieces = "PRNBQK";
    if (switchplayer)
    {
        if (currentPlayer() == WhitePlayer)
        {
            m_copy_currentPlayer = BlackPlayer;
            pieces = "prnbqk";
        }
    }
    else if (!switchplayer)
    {
        if (currentPlayer() == BlackPlayer)
        {
            m_copy_currentPlayer = BlackPlayer;
            pieces = "prnbqk";
        }
    }

    QHash<QPoint, QChar> pos;

    // The current board setup without the outpost which is from.
    for (auto piece: pieces)
    {
        pos.insert(board()->points(piece));
    }

    // Remove any old moves.
    if (!m_moves.empty()) m_moves.clear();

    for (auto iter = pos.constBegin(); iter != pos.constEnd(); ++iter)
    {
        QChar source = iter.value();
        QPoint p = iter.key();
        switch (source.toLatin1())
        {
        case 'P': case 'p':
            setPawnMoves(source, p.x(), p.y());
            break;
        case 'N': case 'n':
            setKnightMoves(source, p.x(), p.y());
            break;
        case 'B': case 'b':
            setBishopMoves(source, p.x(), p.y());
            break;
        case 'R': case 'r':
            setRookMoves(source, p.x(), p.y());
            break;
        case 'Q': case 'q':
            setQueenMoves(source, p.x(), p.y());
            break;
        case 'K': case 'k':
            setKingMoves(source, p.x(), p.y());
            break;
        }
    }

    // Revert.
    if (enpassentWhite != m_enpassent_white)
        m_enpassent_white = enpassentWhite;
    if (enpassentBlack != m_enpassent_black)
        m_enpassent_black = enpassentBlack;
    if (shortcastleWhite != m_shortcastle_white)
        m_shortcastle_white = shortcastleWhite;
    if (longcastleWhite != m_longcastle_white)
        m_longcastle_white = longcastleWhite;

    if (shortcastle_white != m_shortcastle_white)
        m_shortcastle_white = shortcastle_white;
    if (longcastle_white != m_longcastle_white)
        m_longcastle_white = longcastle_white;
    if (shortcastle_black != m_shortcastle_black)
        m_shortcastle_black = shortcastle_black;
    if (longcastle_black != m_longcastle_black)
        m_longcastle_black = longcastle_black;


    // Get the current position of Black King.
    QPoint p = board()->point('k');
    if (m_copy_currentPlayer == BlackPlayer)
        p = board()->point('K');
    QString kingPos =  QChar('a' + p.y() - 1) + QString::number(p.x());

    bool check = false;
    const QList keys = m_moves.keys();

    // Removed warning based on https://stackoverflow.com/questions/35811053/using-c11-range-based-for-loop-correctly-in-qt
    for (auto&&item: keys)
    {
        // We need the current position of the King.
        if (item.contains(kingPos))
        {
            qDebug() << "Check!";
            check = true;
            break;
        }
    }

    // Revert the player.
    if (switchplayer)
    {
        if (m_copy_currentPlayer == BlackPlayer)
        {
            m_copy_currentPlayer = WhitePlayer;
        }
    }

    return check;
}

bool ChessAlgorithm::checkMate()
{
    QString pieces = "prnbqk";
    if (currentPlayer() == BlackPlayer)
    {
        pieces = "PRNBQK";
    }

    QHash<QPoint, QChar> pos;
    for (auto piece: pieces)
    {
        pos.insert(board()->points(piece));
    }

    if (!m_moves.empty()) m_moves.clear();
    for (auto iter = pos.constBegin(); iter != pos.constEnd(); ++iter)
    {
        QChar source = iter.value();
        QPoint from = iter.key();
        switch (source.toLatin1())
        {
        case 'P': case 'p':
            setPawnMoves(source, from.x(), from.y());
            break;
        case 'N': case 'n':
            setKnightMoves(source, from.x(), from.y());
            break;
        case 'B': case 'b':
            setBishopMoves(source, from.x(), from.y());
            break;
        case 'R': case 'r':
            setRookMoves(source, from.x(), from.y());
            break;
        case 'Q': case 'q':
            setQueenMoves(source, from.x(), from.y());
            break;
        case 'K': case 'k':
            setKingMoves(source, from.x(), from.y());
            break;
        }

        QHash<QString, bool> m_potential_moves = m_moves;
        if (!m_moves.empty()) m_moves.clear();

        // Get the current position of the piece.
        for (auto i=m_potential_moves.cbegin(), end=m_potential_moves.cend(); i != end; i++)
        {
            // Do the move and call check().
            QPoint to = toCoordinates(i.key());
            qDebug() << "From " << from << " to " << to;

            // Do the "UNREAL" move.
            QChar pieceOnToSource = board()->data(to.x(), to.y());
            board()->setDataInternal(to.x(), to.y(), board()->data(from.x(), from.y()));
            board()->setDataInternal(from.x(), from.y(), ' ');

            // Get all current positions of the current player.
            pieces = "PRNBQK";
            if (currentPlayer() == BlackPlayer)
            {
                pieces = "prnbqk";
            }
            QHash<QPoint, QChar> otherPos;
            for (auto piece: pieces)
            {
                otherPos.insert(board()->points(piece));
            }
            // For each of these positions get all possible moves and check if that move delivers a check.
            // If not, this can't be a checkmate.
            for (auto iter = otherPos.constBegin(); iter != otherPos.constEnd(); ++iter)
            {
                source = iter.value();
                QPoint pt = iter.key();
                switch (source.toLatin1())
                {
                case 'P': case 'p':
                    setPawnMoves(source, pt.x(), pt.y());
                    break;
                case 'N': case 'n':
                    setKnightMoves(source, pt.x(), pt.y());
                    break;
                case 'B': case 'b':
                    setBishopMoves(source, pt.x(), pt.y());
                    break;
                case 'R': case 'r':
                    setRookMoves(source, pt.x(), pt.y());
                    break;
                case 'Q': case 'q':
                    setQueenMoves(source, pt.x(), pt.y());
                    break;
                case 'K': case 'k':
                    setKingMoves(source, pt.x(), pt.y());
                    break;
                }
            }

            // Get the current position of Black King.
            QPoint p = board()->point('k');
            if (currentPlayer() == BlackPlayer)
                p = board()->point('K');
            QString kingPos =  QChar('a' + p.y() - 1) + QString::number(p.x());

            QList keyList = m_moves.keys();
            if (!keyList.contains("Qx"+kingPos) && !keyList.contains("Px"+kingPos) && !keyList.contains("Nx"+kingPos)
                && !keyList.contains("Rx"+kingPos) && !keyList.contains("Bx"+kingPos))
            {
                board()->setDataInternal(from.x(), from.y(), board()->data(to.x(), to.y()));
                board()->setDataInternal(to.x(), to.y(), ' ');
                if (!m_moves.empty()) m_moves.clear();

                return false;
            }

            // Revert the "UNREAL" move.
            // Check if there was a piece on that spot and get the source.
            board()->setDataInternal(from.x(), from.y(), board()->data(to.x(), to.y()));
            board()->setDataInternal(to.x(), to.y(), pieceOnToSource);

            if (!m_moves.empty()) m_moves.clear();
            if (!keyList.empty()){
                keyList.clear();
            }
        }

        // Clear
        if (!m_potential_moves.empty()) m_potential_moves.clear();
    }

    return true;
}
