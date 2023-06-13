#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QObject>
#include <QHash>

// Datastructure that contains the chess board mappings.
class ChessBoard : public QObject
{
    // Use macro so moc can produce a C++ source file containing the meta-object code for QObject class.
    // Pre-compiler now knows that this class needs to run through moc (Meta Object Compiler).
    Q_OBJECT

    // These properties behave like class data members.
    // Define the ranks property where ranks refers to const method and ranksChanges is the signal that is emitted

    // whenever ranks value is changed.
    Q_PROPERTY(int ranks READ ranks NOTIFY ranksChanged)

    // Define the columns property.
    Q_PROPERTY(int columns READ columns NOTIFY columnsChanged)

    // Property that contains the total number of moves played on the board.
    Q_PROPERTY(int nrOfMoves READ nrOfMoves NOTIFY nrOfMovesChanged)

    int m_ranks;
    int m_columns;
    int m_nrOfMoves;

    // Vector of characters where the character represents the chess piece.
    // A space character means the field is emtpy.
    QVector<char> m_boardData;

public:
    // Don't allow for implicit conversions.
    explicit ChessBoard(int ranks=8, int columns=8, QObject *parent = nullptr);

    // Getter methods.
    int ranks() const;
    int columns() const;
    int nrOfMoves() const;

    char data(int column, int rank) const;

    // Allow to set data on board.
    void setData(int column, int rank, char value);

    // Allow to move piece.
    void movePiece(int fromColumn, int fromRank, int toColumn, int toRank);

    // Method to set chess pieces on board according to FEN notation.
    void setFen(const QString &fen);
    QString getFen();


signals:
    void ranksChanged(int);
    void columnsChanged(int);
    void nrOfMovesChanged(int);

    // Signal when something changed on the board.
    void dataChanged(int c, int r);

    // Signal to reset the board.
    void boardReset();

protected:
    // This can be kept part of the Chessboard class only.
    // No need to make this public, but allow to overwrite.
    void setRanks(int newRanks);
    void setColumns(int newColumns);
    void setNrOfMoves(int nr);

    // Initialises an empty chess board.
    void initBoard();

    // Actual workhorse that sets the data on the board.
    // Allow to make this overwritable.
    bool setDataInternal(int column, int rank, char value);

};

#endif // CHESSBOARD_H
