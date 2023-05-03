#include "chessview.h"
#include "highlight.h"
#include "fieldhighlight.h"
#include <QPainter>
#include <QMouseEvent>

ChessView::ChessView(QWidget *parent): QWidget{parent} {}

void ChessView::setBoard(ChessBoard *board)
{
    qDebug() << "Checking if board is already set";
    // If chess board already set, do nothing.
    if (m_board == board)
    {
        qDebug() << "Board already set. Bail out.";
        return;
    }

    if (m_board)
    {
        // If m_board already set, then disconnect it.
        // This could be an "old" chess board.
        m_board->disconnect(this);
        qDebug() << "Disconnected board.";
    }

    // Assign the board.
    m_board = board;

    // And connect signals to chess board.
    if (board)
    {
        connect(board, SIGNAL(dataChanged(int,int)), this, SLOT(update()));
        connect(board, SIGNAL(boardreset()), this, SLOT(update()));
    }

    // Notify the layout system that this widget has changed and may need to change geomtry.
    // Draw pieces on board.
    updateGeometry();
    qDebug() << "Updated geometry.";
}

ChessBoard *ChessView::board() const
{
    return m_board;
}

QSize ChessView::fieldSize() const
{
    return m_fieldSize;
}

void ChessView::setFieldSize(const QSize &newFieldSize)
{
    if (m_fieldSize == newFieldSize)
        return;

    m_fieldSize = newFieldSize;
    emit fieldSizeChanged();
    updateGeometry();
}

QSize ChessView::sizeHint() const
{
    if (!m_board) return QSize(200, 200);

    QSize boardSize = QSize(fieldSize().width() * m_board->columns() + 1, fieldSize().height() * m_board->ranks() + 1);
    int rankSize = fontMetrics().maxWidth() + 10;
    int columnSize = fontMetrics().height() + 10;

    return boardSize + QSize(rankSize, columnSize);
}

QRect ChessView::fieldRect(int column, int rank) const
{
    if (!m_board) return QRect();

    // Get the field size which is set in the constructor.
    // This will be the QSize of our rectangle on the board.
    const QSize fs = fieldSize();

    // Start at 0 index.
    QRect fRect = QRect(
        QPoint((column - 1) * fs.width(), (m_board->ranks() - rank) * fs.height()), fs
        );

    // Offset rect by rank symbols.
    int offset = fontMetrics().maxWidth();

    return fRect.translated(offset + 10, 10);
}

QPoint ChessView::fieldAt(const QPoint &pt) const
{
    if (!m_board) return QPoint();

    const QSize fs = fieldSize();
    int offset = fontMetrics().maxWidth();

    if (pt.x() < offset) return QPoint();

    int c = (pt.x() - offset) / fs.width();
    int r = pt.y() / fs.height();
    if (c < 0 || c >= m_board->columns() || r < 0 || r >= m_board->ranks())
        return QPoint();

    return QPoint(c + 1, m_board->ranks() - r);
}

void ChessView::paintEvent(QPaintEvent *event)
{
    if (!m_board) return;

    // Instantiate the painter on the widget.
    // this is a reference to ChessView instance.
    QPainter painter(this);

    for (int r=m_board->ranks(); r > 0; --r)
    {
        // Save current Painter state.
        painter.save();

        // Change the state of the painter.
        drawRank(&painter, r);

        // Restore Painter to last saved state.
        painter.restore();
    }
    for (int c=1; c <= m_board->columns(); ++c)
    {
        painter.save();
        drawColumn(&painter, c);
        painter.restore();
    }
    for (int r=1; r <= m_board->ranks(); ++r)
    {
        for (int c=1; c <= m_board->columns(); ++c)
        {
            painter.save();
            drawField(&painter, c, r);
            painter.restore();
        }
    }

    // Highlight the square before the piece is rendered.
    drawHighlights(&painter);

    // Highlight the possible moves squares.

    // Render the pieces.
    for (int r=m_board->ranks(); r > 0 ; --r)
    {
        for (int c=1; c <= m_board->columns(); ++c)
        {
           drawPiece(&painter, c, r);
        }
    }
}

void ChessView::drawRank(QPainter *painter, int rank)
{
    // Start at the first field.
    QRect r = fieldRect(1, rank);
    QRect rankRect = QRect(
                         0, r.top(), r.left(),
                         r.height()
                         ).adjusted(2, 0, -2, 0);

    // Ranks are represented by numbers: [1-8].
    QString rankText = QString::number(rank);
    painter->drawText(rankRect, Qt::AlignVCenter|Qt::AlignRight, rankText);
}

void ChessView::drawColumn(QPainter *painter, int column)
{
    QRect r = fieldRect(column, 1);
    QRect columnRect = QRect(r.left(), r.bottom(), r.width(), height() - r.bottom()).adjusted(0, 2, 0, -2);

    // Columns are labeled by small chars [a-h].
    painter->drawText(columnRect, Qt::AlignHCenter|Qt::AlignTop, QChar('a' + column - 1));
}

void ChessView::drawField(QPainter *painter, int column, int rank)
{
    QRect rect = fieldRect(column, rank);
    QColor fillColor = (column + rank) % 2 ? palette().color(QPalette::Light) : palette().color(QPalette::Mid);
    painter->setPen(palette().color(QPalette::Dark));
    painter->setBrush(fillColor);
    painter->drawRect(rect);
}

void ChessView::setPiece(char type, const QIcon &icon)
{
    m_pieces.insert(type, icon);
    update();
}

QIcon ChessView::piece(char type) const
{
    return m_pieces.value(type, QIcon());
}

void ChessView::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint pt = fieldAt(event->pos());
    if (pt.isNull()) return;
    emit clicked(pt);
}

void ChessView::drawPiece(QPainter *painter, int column, int rank)
{

    QRect rect = fieldRect(column, rank);

    char value = m_board->data(column, rank);

    // Check if we are on empty field.
    if (value != ' ')
    {
        QIcon icon = piece(value);
        if (!icon.isNull())
        {
            icon.paint(painter, rect, Qt::AlignCenter);
        }
    }
}

void ChessView::addHighlight(Highlight *hl)
{
    m_highlights.append(hl);
    update();
}

void ChessView::removeHighlight(Highlight *hl)
{
    m_highlights.removeOne(hl);
    update();
}

void ChessView::drawHighlights(QPainter *painter)
{
    // Get the highlights.
    for (int idx=0; idx < highlightCount(); ++idx)
    {
        Highlight *hl = highlight(idx);
        if (hl->type() == FieldHighlight::Type)
        {
            FieldHighlight *fhl = static_cast<FieldHighlight*>(hl);
            QRect rect = fieldRect(fhl->column(), fhl->rank());
            painter->fillRect(rect, fhl->color());
        }
    }
}
