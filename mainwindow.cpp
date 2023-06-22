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
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_view = new ChessView(this);

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

    m_lblCheck = new QLabel(this);
    m_lblCheck->move(880, 60);
    m_lblCheck->resize(250, 60);

    // Set font for label.
    QFont fontCheck = m_lblCheck->font();
    fontCheck.setPointSize(30);
    fontCheck.setBold(true);
    m_lblCheck->setFont(fontCheck);
    m_lblCheck->show();

    // Label to display current player.
    m_lblPlayer = new QLabel(this);
    m_lblPlayer->move(860, 100);
    m_lblPlayer->resize(250, 40);

    // Set font for label.
    QFont font = m_lblPlayer->font();
    font.setPointSize(30);
    font.setBold(true);
    m_lblPlayer->setFont(font);
    m_lblPlayer->setText("Wit aan zet!");
    m_lblPlayer->show();

    // Connect SIGNAL current player changed so we can display current player in UI.
    connect(m_algorithm, &ChessAlgorithm::currentPlayerChanged, this, &MainWindow::playerChanged);

    // Dummy label indicating player moves.
    QLabel *m_lbl = new QLabel(this);
    m_lbl->move(900, 180);
    m_lbl->resize(200, 20);
    m_lbl->setText("Zetten van spelers");
    m_lbl->show();

    // List contains player moves.
    m_lstMoves = new QListWidget(this);
    m_lstMoves->move(860, 200);
    m_lstMoves->resize(200, 180);
    m_lstMoves->show();

    // Connect SIGNAL for move so we can display move in UI.
    connect(m_algorithm->board(), &ChessBoard::nrOfMovesChanged, this, &MainWindow::updateList);

    // Dummy label indicating engine moves.
    QLabel *m_lblUci = new QLabel(this);
    m_lblUci->move(900, 400);
    m_lblUci->resize(200, 20);
    m_lblUci->setText("Zetten van computer");
    m_lblUci->show();

    // List that contains the computer moves.
    m_lstCompMoves = new QListWidget(this);
    m_lstCompMoves->move(860, 420);
    m_lstCompMoves->resize(200, 180);
    m_lstCompMoves->show();

    // Set first engine move.
    m_algorithm->board()->setNrOfEngMoves(1);

    // Capture the event when the board is clicked
    connect(m_view, &ChessView::clicked, this, &MainWindow::viewClicked);

    // Uncheck listener.
    connect(m_algorithm, &ChessAlgorithm::unChecked, this, &MainWindow::unCheck);

    // This needs to connect to the UCI engine.
    connect(m_algorithm->engine(), &UciEngine::engineMove, this, &MainWindow::updateBestMoveList);

    // Listen to castling.
    connect(m_algorithm->board(), &ChessBoard::whiteHasCastled, m_algorithm, &ChessAlgorithm::whiteCastle);

    // Connect SIGNAL when there is checkmate or stale mate.
    connect(m_algorithm, &ChessAlgorithm::gameOver, this, &MainWindow::gameOver);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateList()
{
    QString move = m_algorithm->currentMove();
    int nr = m_algorithm->board()->nrOfMoves();

    // Add a new row with white's move.
    if (nr > 0 && nr % 2 == 1)
    {
        m_lstMoves->addItem(QString::number((nr + 1)/2) + ".  " + move + "\t");
    }
    // Add black's move.
    if (nr > 0 && nr % 2 == 0)
    {
        QListWidgetItem *item = m_lstMoves->item((nr - 1)/2);
        item->setText(item->text() + move);
    }
}

void MainWindow::updateBestMoveList(QString move)
{
    qInfo() << Q_FUNC_INFO;

    int nr = m_algorithm->board()->nrOfEngMoves();

    // Add a new row with white's move.
    if (nr > 0 && nr % 2 == 1)
    {
        m_lstCompMoves->addItem(QString::number((nr + 1)/2) + ".  " + move + "\t");
    }
    // Add black's move.
    if (nr > 0 && nr % 2 == 0)
    {
        QListWidgetItem *item = m_lstCompMoves->item((nr - 1)/2);
        item->setText(item->text() + move);
    }
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

    qInfo() << Q_FUNC_INFO;


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

            /*if (m_view->board()->blackChecked())
            {
                m_possibleField = new FieldHighlight(5, 8, QColor(244,198,198), FieldHighlight::Rectangle);
                m_view->addHighlight(m_possibleField);
            }*/
            connect(m_algorithm, &ChessAlgorithm::checked, this, &MainWindow::highlightCheck);

            // Highlight possible moves for selected piece.
            m_algorithm->setMoves(field.x(), field.y());
            m_algorithm->setEngineMoves( m_algorithm->getFENBoard());

            // Get the possible moves from the algorithm.
            QHash<QString, bool> m_moves = m_algorithm->getMoves();
            QHash<QString, bool>::iterator iter;
            for (iter = m_moves.begin(); iter != m_moves.end(); ++iter)
            {
                qDebug() << iter.key() << ": " << iter.value();
                QPoint p = m_algorithm->toCoordinates(iter.key());
                if (iter.key().contains("x"))
                {
                    m_possibleField = new FieldHighlight(p.x(), p.y(), QColor(250,244,220), FieldHighlight::Rectangle);
                }
                else
                {
                    m_possibleField = new FieldHighlight(p.x(), p.y(), QColor(250,244,220), FieldHighlight::Circle);
                }
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

void MainWindow::highlightCheck(const QPoint &p)
{
    qInfo() << Q_FUNC_INFO;

    m_lblCheck->setText("Schaak!");

    QString move = m_algorithm->currentMove();
    move = move + '+';
    int nr = m_algorithm->board()->nrOfMoves();
    qDebug() << "Move :" << nr;

    // Here we alter the move to add the check FEN sign in UI.
    if (nr > 0 && nr % 2 == 1)
    {
        int itemIndex = (nr + 1)/2;
        QListWidgetItem *item = m_lstMoves->item(itemIndex - 1);
        item->setText(QString::number(itemIndex) + ".  " + move + "\t");
    }

    emit m_view->clicked(p);
}

void MainWindow::unCheck()
{
    qInfo() << Q_FUNC_INFO;;

    m_lblCheck->setText("");
}

void MainWindow::gameOver(ChessAlgorithm::Result result)
{
    QString text;
    switch(result) {
    case ChessAlgorithm::WhiteWin: text = "White wins!"; break;
    case ChessAlgorithm::BlackWin: text = "Black wins!"; break;
    default:
        text = "It's a draw";
    }

    QMessageBox::information(this, "Game over", QStringLiteral("The game has ended. %1").arg(text));
}
