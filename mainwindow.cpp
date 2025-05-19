#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Create widgets
    transactionTable = new QTableWidget(0, 4);
    transactionTable->setHorizontalHeaderLabels({"ID", "Status", "Held Locks", "Waiting For"});
    dataItemTable = new QTableWidget(0, 2);
    dataItemTable->setHorizontalHeaderLabels({"ID", "Lock Holder"});
    graphScene = new QGraphicsScene;
    graphView = new QGraphicsView(graphScene);
    logText = new QTextEdit;
    logText->setReadOnly(true);
    tidInput = new QLineEdit;
    didInput = new QLineEdit;
    addTransactionBtn = new QPushButton("Add Transaction");
    addDataItemBtn = new QPushButton("Add Data Item");
    requestLockBtn = new QPushButton("Request Lock");
    releaseLockBtn = new QPushButton("Release Lock");
    detectDeadlockBtn = new QPushButton("Detect Deadlock");
    recoverBtn = new QPushButton("Recover");
    preventionCheckBox = new QCheckBox("Enable Prevention");

    // Set up layout
    QWidget *centralWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(new QLabel("Transactions"));
    mainLayout->addWidget(transactionTable);
    mainLayout->addWidget(new QLabel("Data Items"));
    mainLayout->addWidget(dataItemTable);
    mainLayout->addWidget(new QLabel("Wait-For Graph"));
    mainLayout->addWidget(graphView);
    mainLayout->addWidget(new QLabel("Log"));
    mainLayout->addWidget(logText);
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(new QLabel("TID:"));
    inputLayout->addWidget(tidInput);
    inputLayout->addWidget(new QLabel("DID:"));
    inputLayout->addWidget(didInput);
    inputLayout->addWidget(addTransactionBtn);
    inputLayout->addWidget(addDataItemBtn);
    inputLayout->addWidget(requestLockBtn);
    inputLayout->addWidget(releaseLockBtn);
    inputLayout->addWidget(detectDeadlockBtn);
    inputLayout->addWidget(recoverBtn);
    inputLayout->addWidget(preventionCheckBox);
    mainLayout->addLayout(inputLayout);
    setCentralWidget(centralWidget);
    setWindowTitle("Predict, Prevent and Proceed: Deadlock Handling");

    // Connect signals
    connect(addTransactionBtn, &QPushButton::clicked, this, &MainWindow::addTransaction);
    connect(addDataItemBtn, &QPushButton::clicked, this, &MainWindow::addDataItem);
    connect(requestLockBtn, &QPushButton::clicked, this, &MainWindow::requestLock);
    connect(releaseLockBtn, &QPushButton::clicked, this, &MainWindow::releaseLock);
    connect(detectDeadlockBtn, &QPushButton::clicked, this, &MainWindow::detectDeadlock);
    connect(recoverBtn, &QPushButton::clicked, this, &MainWindow::recover);
    connect(preventionCheckBox, &QCheckBox::toggled, this, &MainWindow::togglePrevention);
}

MainWindow::~MainWindow() {}

void MainWindow::addTransaction() {
    int tid = lockManager.addTransaction();
    logText->append("Added transaction T" + QString::number(tid));
    refreshTables();
    refreshGraph();
}

void MainWindow::addDataItem() {
    int did = lockManager.addDataItem();
    logText->append("Added data item D" + QString::number(did));
    refreshTables();
    refreshGraph();
}

void MainWindow::requestLock() {
    bool tidOk, didOk;
    int tid = tidInput->text().toInt(&tidOk);
    int did = didInput->text().toInt(&didOk);
    if (!tidOk || !didOk) {
        logText->append("Invalid TID or DID");
        return;
    }
    std::string log;
    lockManager.requestLock(tid, did, log);
    logText->append(QString::fromStdString(log));
    refreshTables();
    refreshGraph();
}

void MainWindow::releaseLock() {
    bool tidOk, didOk;
    int tid = tidInput->text().toInt(&tidOk);
    int did = didInput->text().toInt(&didOk);
    if (!tidOk || !didOk) {
        logText->append("Invalid TID or DID");
        return;
    }
    std::string log;
    lockManager.releaseLock(tid, did, log);
    logText->append(QString::fromStdString(log));
    refreshTables();
    refreshGraph();
}

void MainWindow::detectDeadlock() {
    std::string log;
    lockManager.detectDeadlock(log, currentCycle);
    logText->append(QString::fromStdString(log));
}

void MainWindow::recover() {
    std::string log;
    lockManager.recover(currentCycle, log);
    logText->append(QString::fromStdString(log));
    refreshTables();
    refreshGraph();
}

void MainWindow::togglePrevention(bool checked) {
    lockManager.setPreventionEnabled(checked);
    logText->append("Prevention " + QString(checked ? "enabled" : "disabled"));
}

void MainWindow::refreshTables() {
    const auto& transactions = lockManager.getTransactions();
    transactionTable->setRowCount(transactions.size());
    for (size_t i = 0; i < transactions.size(); ++i) {
        const auto& T = transactions[i];
        transactionTable->setItem(i, 0, new QTableWidgetItem(QString::number(T.id)));
        transactionTable->setItem(i, 1, new QTableWidgetItem(T.active ? "Active" : "Terminated"));
        QString heldLocks;
        for (int did : T.heldLocks) heldLocks += "D" + QString::number(did) + " ";
        transactionTable->setItem(i, 2, new QTableWidgetItem(heldLocks));
        QString waitingFor;
        for (int did : T.waitingFor) waitingFor += "D" + QString::number(did) + " ";
        transactionTable->setItem(i, 3, new QTableWidgetItem(waitingFor));
    }

    const auto& dataItems = lockManager.getDataItems();
    const auto& lockTable = lockManager.getLockTable();
    dataItemTable->setRowCount(dataItems.size());
    for (size_t i = 0; i < dataItems.size(); ++i) {
        int did = dataItems[i];
        dataItemTable->setItem(i, 0, new QTableWidgetItem(QString::number(did)));
        int holder = lockTable.at(did);
        dataItemTable->setItem(i, 1, new QTableWidgetItem(holder == -1 ? "-" : "T" + QString::number(holder)));
    }
}

void MainWindow::refreshGraph() {
    graphScene->clear();
    const auto& transactions = lockManager.getTransactions();
    const auto& lockTable = lockManager.getLockTable();
    if (transactions.empty()) return;

    double radius = 100;
    double angleStep = 2 * M_PI / transactions.size();
    std::map<int, QPointF> positions;

    // Place nodes
    for (size_t i = 0; i < transactions.size(); ++i) {
        if (!transactions[i].active) continue;
        double angle = i * angleStep;
        double x = radius * cos(angle);
        double y = radius * sin(angle);
        positions[transactions[i].id] = QPointF(x, y);
        auto *node = graphScene->addEllipse(x - 10, y - 10, 20, 20);
        node->setBrush(Qt::lightGray);
        auto *text = graphScene->addText("T" + QString::number(transactions[i].id));
        text->setPos(x - 5, y - 5);
    }

    // Draw edges
    std::map<int, std::set<int>> waitForGraph;
    for (const auto& T : transactions) {
        if (T.active && !T.waitingFor.empty()) {
            for (int did : T.waitingFor) {
                auto it = lockTable.find(did);
                if (it != lockTable.end() && it->second != -1 && it->second != T.id) {
                    waitForGraph[T.id].insert(it->second);
                }
            }
        }
    }
    for (const auto& pair : waitForGraph) {
        int from = pair.first;
        QPointF fromPos = positions[from];
        for (int to : pair.second) {
            QPointF toPos = positions[to];
            graphScene->addLine(fromPos.x(), fromPos.y(), toPos.x(), toPos.y(), QPen(Qt::blue));
        }
    }
}
