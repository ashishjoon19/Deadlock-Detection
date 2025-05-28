#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QGraphicsView>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDialog>
#include "LockManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    LockManager lockManager;
    QTableWidget *transactionTable;
    QTableWidget *dataItemTable;
    QGraphicsView *graphView;
    QGraphicsScene *graphScene;
    QTextEdit *logText;
    QLineEdit *tidInput, *didInput;
    QPushButton *addTransactionBtn, *addDataItemBtn, *requestLockBtn, *releaseLockBtn,
        *detectDeadlockBtn, *recoverBtn;
    QCheckBox *preventionCheckBox;
    std::vector<int> currentCycle;
    // Full-screen graph components
    QDialog *graphDialog;
    QGraphicsView *fullScreenGraphView;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addTransaction();
    void addDataItem();
    void requestLock();
    void releaseLock();
    void detectDeadlock();
    void recover();
    void togglePrevention(bool checked);
    void onFullScreenGraph(); // Slot for full-screen graph button

private:
    void refreshTables();
    void refreshGraph();
};

#endif // MAINWINDOW_H