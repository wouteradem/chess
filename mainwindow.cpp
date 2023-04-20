#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "chessalgorithm.h"
#include "chessview.h"
#include "fieldhighlight.h"
#include <QLayout>

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
    m_view->setFieldSize(QSize(50, 50));

    //m_selectedField = 0;

    connect(m_view, SIGNAL(clicked(QPoint)), this, SLOT(viewClicked(QPoint)));

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::viewClicked(const QPoint &field)
{
    if (m_clickPoint.isNull())
    {
        if (m_view->board()->data(field.x(), field.y()) != ' ')
        {
            m_clickPoint = field;
            m_selectedField = new FieldHighlight(field.x(), field.y(), QColor(255, 0, 0, 50));
            m_view->addHighlight(m_selectedField);
        }
    }
    else
    {
        if (field != m_clickPoint)
        {
            m_view->board()->movePiece(m_clickPoint.x(), m_clickPoint.y(), field.x(), field.y());
        }
        m_clickPoint = QPoint();
        m_view->removeHighlight(m_selectedField);
        delete m_selectedField;
        m_selectedField = 0;
    }
}



