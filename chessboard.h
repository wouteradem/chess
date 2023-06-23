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

    // Board properties.
    Q_PROPERTY(int ranks MEMBER m_ranks READ ranks NOTIFY ranksChanged)
    Q_PROPERTY(int columns MEMBER m_columns READ columns NOTIFY columnsChanged)

    // Number of moves property.
    Q_PROPERTY(int nrOfMoves MEMBER m_nrOfMoves READ nrOfMoves WRITE setNrOfMoves NOTIFY nrOfMovesChanged)
    //Q_PROPERTY(int nrOfEngMoves MEMBER m_nrOfEngMoves READ nrOfEngMoves WRITE setNrOfEngMoves NOTIFY nrOfEngMovesChanged)

    // Castle properties.
    Q_PROPERTY(CastleType whiteCasteled MEMBER m_whiteCastled READ whiteCastled WRITE setWhiteCastled NOTIFY whiteHasCastled)
    Q_PROPERTY(CastleType blackCasteled MEMBER m_blackCastled READ blackCastled WRITE setBlackCastled NOTIFY blackHasCastled)

    Q_PROPERTY(bool whiteChecked MEMBER m_whiteChecked READ whiteChecked WRITE setWhiteChecked NOTIFY whiteIsChecked)
    Q_PROPERTY(bool blackChecked MEMBER m_blackChecked READ blackChecked WRITE setBlackChecked NOTIFY blackIsChecked)

public:
    explicit ChessBoard(int ranks=8, int columns=8, QObject *parent = nullptr);

    enum CastleType {Short, Long, None};
    Q_ENUM(CastleType)

    // Getter methods.
    int ranks() const;
    int columns() const;
    int nrOfMoves() const;
    int nrOfEngMoves() const;
    bool whiteChecked() const;
    bool blackChecked() const;
    CastleType whiteCastled() const;
    CastleType blackCastled() const;

    // Setter methods.
    void setWhiteCastled(CastleType type);
    void setBlackCastled(CastleType type);
    void setNrOfMoves(int nr);
    void setNrOfEngMoves(int nr);
    void setWhiteChecked(bool isChecked);
    void setBlackChecked(bool isChecked);

    QChar data(int column, int rank) const;
    QPoint point(QChar piece) const;
    QHash<QPoint, QChar> points(QChar piece) const;
    void setData(int column, int rank, QChar value);
    void movePiece(int fromColumn, int fromRank, int toColumn, int toRank);

    void setFen(const QString &fen);
    QString getFen(const QChar player);
    bool setDataInternal(int column, int rank, QChar value);

signals:
    void ranksChanged(int);
    void columnsChanged(int);
    void nrOfMovesChanged();
    void nrOfEngMovesChanged();
    void whiteHasCastled(CastleType);
    void blackHasCastled(CastleType);
    void dataChanged(int c, int r);
    void boardReset();
    void whiteIsChecked();
    void blackIsChecked();

protected:
    void setRanks(int newRanks);
    void setColumns(int newColumns);

    // Initialises an empty chess board.
    void initBoard();

private:
    int m_ranks;
    int m_columns;
    int m_nrOfMoves;
    int m_nrOfEngMoves;
    CastleType m_whiteCastled;
    CastleType m_blackCastled;
    bool m_whiteChecked;
    bool m_blackChecked;

    QVector<QChar> m_boardData;
};

#endif // CHESSBOARD_H
