#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QObject>

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

    int m_ranks;
    int m_columns;

    // Vector of characters where the character represents the chess piece.
    // We use FEN notation.
    // A space character means the field is emtpy.
    QVector<char> m_boardData;

public:
    // Don't allow for implicit conversions.
    explicit ChessBoard(int ranks=8, int columns=8, QObject *parent = nullptr);

    // Getter methods.
    int ranks() const;
    int columns() const;

    char data(int column, int rank) const;

    // Allow to set data on board.
    void setData(int column, int rank, char value);

    // Allow to move piece.
    void movePiece(int fromColumn, int fromRank, int toColumn, int toRank);


    void setFen(const QString &fen);

signals:
    // Signals for the QPROPERTY.
    void ranksChanged(int);
    void columnsChanged(int);

    // Signal when something changed on the board.
    void dataChanged(int c, int r);

    // Signal to reset the board.
    void boardreset();

protected:
    // This can be kept part of the Chessboard class only.
    // No need to make this public, but allow to overwrite.
    void setRanks(int newRanks);
    void setColumns(int newColumns);

    // Initialises an empty chess board.
    void initBoard();

    // Actual workhorse that sets the data on the board.
    // Allow to make this overwritable.
    bool setDataInternal(int column, int rank, char value);
};

#endif // CHESSBOARD_H