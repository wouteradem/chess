#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chessalgorithm.h"
#include "chessview.h"
#include "fieldhighlight.h"
#include <QObject>
#include <QLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_view = new ChessView(this);

    // Capture the event when the board is clicked
    connect(m_view, &ChessView::clicked, this, &MainWindow::viewClicked);

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

    m_view->resize(900, 1000);
    m_view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_view->setFieldSize(QSize(100, 100));
    m_view->show();

    // Label to display current player.
    m_lblPlayer = new QLabel(this);
    m_lblPlayer->move(860, 100);
    m_lblPlayer->resize(200, 20);
    m_lblPlayer->setText("Wit aan zet!");
    m_lblPlayer->show();

    // Connect the SIGNAL current player changed so we can update the UI.
    connect(m_algorithm, &ChessAlgorithm::currentPlayerChanged, this, &MainWindow::playerChanged);

    // Dummy label indicating moves of players.
    QLabel *m_lbl = new QLabel(this);
    m_lbl->move(860, 180);
    m_lbl->resize(200, 20);
    m_lbl->setText("Zetten van spelers");
    m_lbl->show();

    // List that contains the player moves.
    m_lstMoves = new QListWidget(this);
    m_lstMoves->move(860, 200);
    m_lstMoves->resize(200, 180);
    m_lstMoves->show();

    // Connect the SIGNAL board changed so we can update the UI.
    //connect(m_algorithm, &ChessAlgorithm::boardChanged, this, &MainWindow::viewClicked);

    // Dummy label indicating moves of computer.
    QLabel *m_lblUci = new QLabel(this);
    m_lblUci->move(860, 400);
    m_lblUci->resize(200, 20);
    m_lblUci->setText("Zetten van computer");
    m_lblUci->show();

    // List that contains the computer moves.
    m_lstCompMoves = new QListWidget(this);
    m_lstCompMoves->move(860, 420);
    m_lstCompMoves->resize(200, 180);
    m_lstCompMoves->show();

    // This needs to connect to the UCI engine.
    //m_lstBestMoves->connect(m_lstBestMoves, SIGNAL(addItem()), this, SLOT(viewClicked()));

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::playerChanged()
{
    qInfo() << Q_FUNC_INFO;
    ChessAlgorithm::Player player = m_algorithm->currentPlayer();
    if (player == ChessAlgorithm::BlackPlayer)
    {
        m_lblPlayer->setText("Zwart aan zet!");
    }
    else if (player == ChessAlgorithm::WhitePlayer)
    {
        m_lblPlayer->setText("Wit aan zet!");
    }
    else
    {
        m_lblPlayer->setText("");
    }
}

void MainWindow::viewClicked(const QPoint &field)
{
    // Did the user click somewhere?
    if (m_clickPoint.isNull())
    {
        // Only allow to select chess pieces.
        if (m_view->board()->data(field.x(), field.y()) != ' ')
        {
            m_clickPoint = field;

            // Highlight the selected piece.
            m_selectedField = new FieldHighlight(field.x(), field.y(), QColor(246, 246, 132), FieldHighlight::Rectangle);
            m_view->addHighlight(m_selectedField);

            // Highlight possible moves for selected piece.
            m_algorithm->setMoves(field.x(), field.y());
            m_algorithm->setEngineMoves(field.x(), field.y());

            // Get the possible moves from the algorithm.
            QHash<QString, bool> m_moves = m_algorithm->getMoves();
            QHash<QString, bool>::iterator iter;
            for (iter = m_moves.begin(); iter != m_moves.end(); ++iter)
            {
                qDebug() << iter.key() << ": " << iter.value();
                QPoint p = m_algorithm->toCoordinates(iter.key());

                m_possibleField = new FieldHighlight(p.x(), p.y(), QColor(217,216,191), FieldHighlight::Circle);
                m_view->addHighlight(m_possibleField);
            }
        }
    }
    else
    {
        // This is the move we want to do.
        if (field != m_clickPoint)
        {
            m_algorithm->move(m_clickPoint, field);
            // Put the move into the UI.
            //m_lstMoves->addItem("1.   ");
            //m_lstMoves->addItem("2.   c3\t Kc6");

            /*
            QListWidgetItem *item = lstMoves->item(0);
            item->setText(item->text() + "e4");

            item = lstMoves->item(0);
            item->setText(item->text() + "\t e5");
            item->setBackground(Qt::green);
            */
        }

        // Clean up piece highlight.
        m_clickPoint = QPoint();
        m_view->removeHighlight(m_selectedField);

        // Clean up moves highlight.
        int hlCount = m_view->highlightCount();
        for (size_t i=0; i<hlCount; i++)
        {
            m_view->removeHighlight(m_view->highlight(0));
        }
    }
}
