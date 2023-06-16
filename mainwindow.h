#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include "chessview.h"
#include "chessalgorithm.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void viewClicked(const QPoint &field);
    void playerChanged();
    void highlightCheck(const QPoint &field);

private slots:
    void gameOver(ChessAlgorithm::Result);
    void updateList();

private:
    Ui::MainWindow *ui;
    ChessView *m_view;
    QLabel *m_lblPlayer;
    QListWidget *m_lstMoves;
    QListWidget *m_lstCompMoves;

    ChessAlgorithm *m_algorithm;
    QPoint m_clickPoint;
    Highlight *m_selectedField;
    Highlight *m_possibleField;
};

#endif // MAINWINDOW_H
