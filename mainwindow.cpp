#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "chessalgorithm.h"
#include "chessview.h"
#include "fieldhighlight.h"
#include <QObject>
#include <QLayout>
#include <QPushButton>
#include <QListWidget>

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

    // Create new Chess game.
    m_algorithm = new ChessAlgorithm(this);
    m_algorithm->newGame();
    m_view->setBoard(m_algorithm->board());

    setCentralWidget(m_view);
    m_view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_view->setFieldSize(QSize(100, 100));

    // Capture the event when the board is clicked
    connect(m_view, SIGNAL(clicked(QPoint)), this, SLOT(viewClicked(QPoint)));
\
    // TODO: Set non-resizable.
    // TODO: Put this into functions.
    QPushButton *btnReset = new QPushButton(this);
    btnReset->setText("New Game");
    btnReset->resize(200, 50);
    btnReset->move(900, 10);
    btnReset->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    btnReset->connect(btnReset, SIGNAL(clicked()), this, SLOT(btnClicked()));
    btnReset->show();

    QPushButton *btnLoad = new QPushButton(this);
    btnLoad->setText("Load Game");
    btnLoad->resize(200, 50);
    btnLoad->move(900, 70);
    btnLoad->show();

    QPushButton *btnSave = new QPushButton(this);
    btnSave->setText("Save Game");
    btnSave->resize(200, 50);
    btnSave->move(900, 130);
    btnSave->show();

    // Widget to display moves.
    QListWidget *lstMoves = new QListWidget(this);
    lstMoves->move(900, 200);
    lstMoves->resize(200, 500);

    // We want to react on a move.
    //lstMoves->connect(lstMoves, SIGNAL(), this, SLOT(addItem()))

}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::btnClicked()
{
    qDebug() << "I'm clicked!";

    m_algorithm->newGame();
    m_view->setBoard(m_algorithm->board());
}

void MainWindow::viewClicked(const QPoint &field)
{
    qDebug() << "I'm clicked!";
    // Did the user click somewhere?
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
            m_algorithm->setMoves(field.x(), field.y());
        }
    }
    else
    {
        qDebug() << "Want to move a piece!";
        if (field != m_clickPoint)
        {
             m_algorithm->move(m_clickPoint, field);
        }

        // Reset.
        m_clickPoint = QPoint();
        m_view->removeHighlight(m_selectedField);

        // Clean up.
        delete m_selectedField;
        m_selectedField = 0;
    }
}
