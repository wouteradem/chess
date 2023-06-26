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
    void unCheck();

private slots:
    void gameOver(ChessAlgorithm::Result);
    void updateList();
    void updateBestMoveList(QString move);
    void checkYourself();

private:
    Ui::MainWindow *ui;
    QPointer<ChessView> m_view;
    QPointer<QLabel> m_lblPlayer;
    QPointer<QLabel> m_lblCheck;
    QPointer<QListWidget> m_lstMoves;
    QPointer<QListWidget> m_lstCompMoves;

    QPointer<ChessAlgorithm> m_algorithm;
    QPoint m_clickPoint;
    Highlight *m_selectedField;
    Highlight *m_possibleField;
};

#endif // MAINWINDOW_H
