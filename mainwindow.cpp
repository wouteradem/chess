#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "chessalgorithm.h"
#include "chessview.h"
#include "fieldhighlight.h"
#include <QLayout>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_view = new ChessView;
    m_view->setPiece('P', QIcon(":/pieces/Chess_plt45.svg"));
    m_view->setPiece('K', QIcon(":/pieces/Chess_klt45.svg"));
    m_view->setPiece('Q', QIcon(":/pieces/Chess_qlt45.svg"));
    m_view->setPiece('R', QIcon(":/pieces/Chess_rlt45.svg"));
    m_view->setPiece('N', QIcon(":/pieces/Chess_nlt45.svg"));
    m_view->setPiece('B', QIcon(":/pieces/Chess_blt45.svg"));
    m_view->setPiece('p', QIcon(":/pieces/Chess_pdt45.svg"));
    m_view->setPiece('k', QIcon(":/pieces/Chess_kdt45.svg"));
    m_view->setPiece('q', QIcon(":/pieces/Chess_qdt45.svg"));
    m_view->setPiece('r', QIcon(":/pieces/Chess_rdt45.svg"));
    m_view->setPiece('n', QIcon(":/pieces/Chess_ndt45.svg"));
    m_view->setPiece('b', QIcon(":/pieces/Chess_bdt45.svg"));

    m_algorithm = new ChessAlgorithm(this);
    m_algorithm->newGame();

    m_view->setBoard(m_algorithm->board());
    setCentralWidget(m_view);

    m_view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_view->setFieldSize(QSize(100, 100));

    //m_selectedField = 0;

    connect(m_view, SIGNAL(clicked(QPoint)), this, SLOT(viewClicked(QPoint)));
\
    // TODO: Set title.
    // TODO: Set non-resizable.
    // TODO: Need to add Click event to reset chess board.
    QPushButton *btnReset = new QPushButton(m_view);
    btnReset->setText("New Game");
    btnReset->resize(200, 50);
    btnReset->move(900, 10);
    btnReset->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Add another widget to display the moves.

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::viewClicked(const QPoint &field)
{
    if (m_clickPoint.isNull())
    {
        qDebug() << "Click point is null";
        // Only allow to select chess pieces.
        if (m_view->board()->data(field.x(), field.y()) != ' ')
        {
            m_clickPoint = field;

            // Highlight the selected piece.
            m_selectedField = new FieldHighlight(field.x(), field.y(), QColor(246, 246, 132));
            m_view->addHighlight(m_selectedField);

            // Highligt possible moves for selected piece.
            m_algorithm->setPossibleMoves(field.x(), field.y());
        }
    }
    else
    {
        qDebug() << "Want to move a piece!";
        if (field != m_clickPoint)
        {
            //m_view->board()->movePiece(m_clickPoint.x(), m_clickPoint.y(), field.x(), field.y());
            m_algorithm->move(m_clickPoint, field);
        }
        m_clickPoint = QPoint();
        m_view->removeHighlight(m_selectedField);
        delete m_selectedField;
        m_selectedField = 0;
    }
}
