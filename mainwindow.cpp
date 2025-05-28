#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QHeaderView>
#include <QVector2D>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Create widgets
    transactionTable = new QTableWidget(0, 4);
    transactionTable->setHorizontalHeaderLabels({"ID", "Status", "Held Locks", "Waiting For"});
    transactionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    dataItemTable = new QTableWidget(0, 2);
    dataItemTable->setHorizontalHeaderLabels({"ID", "Lock Holder"});
    dataItemTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    graphScene = new QGraphicsScene;
    graphView = new QGraphicsView(graphScene);
    graphView->setRenderHint(QPainter::Antialiasing);
    graphView->setDragMode(QGraphicsView::ScrollHandDrag);
    graphView->setInteractive(true);
    logText = new QTextEdit;
    logText->setReadOnly(true);
    tidInput = new QLineEdit;
    didInput = new QLineEdit;
    addTransactionBtn = new QPushButton("Add Transaction");
    addTransactionBtn->setToolTip("Add a new transaction");
    addDataItemBtn = new QPushButton("Add Data Item");
    addDataItemBtn->setToolTip("Add a new data item");
    requestLockBtn = new QPushButton("Request Lock");
    requestLockBtn->setToolTip("Request a lock for the specified TID on DID");
    releaseLockBtn = new QPushButton("Release Lock");
    releaseLockBtn->setToolTip("Release the lock held by TID on DID");
    detectDeadlockBtn = new QPushButton("Detect Deadlock");
    detectDeadlockBtn->setToolTip("Check for deadlocks in the current state");
    recoverBtn = new QPushButton("Recover");
    recoverBtn->setToolTip("Recover from the detected deadlock by terminating a transaction");
    preventionCheckBox = new QCheckBox("Enable Prevention");
    preventionCheckBox->setToolTip("Enable deadlock prevention by enforcing lock ordering");
    QPushButton *fullScreenGraphBtn = new QPushButton("View Graph Full Screen");
    fullScreenGraphBtn->setToolTip("Open the Wait-For Graph in full screen");

    // Set up layout
    QWidget *centralWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    mainLayout->addWidget(new QLabel("Transactions"));
    mainLayout->addWidget(transactionTable);
    mainLayout->addWidget(new QLabel("Data Items"));
    mainLayout->addWidget(dataItemTable);
    mainLayout->addWidget(new QLabel("Wait-For Graph"));
    mainLayout->addWidget(graphView);
    mainLayout->addWidget(fullScreenGraphBtn);
    mainLayout->addWidget(new QLabel("Log"));
    mainLayout->addWidget(logText);

    // Add user guide labels
    QLabel *guideLabel1 = new QLabel("To add transaction, click on Add Transaction button");
    guideLabel1->setStyleSheet("font-style: italic; color: #555555;");
    mainLayout->addWidget(guideLabel1);

    QLabel *guideLabel2 = new QLabel("To add data, click on Add Data Item button");
    guideLabel2->setStyleSheet("font-style: italic; color: #555555;");
    mainLayout->addWidget(guideLabel2);

    QLabel *controlLabel = new QLabel("Enter TID and DID for lock operations:");
    controlLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(controlLabel);

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
    connect(fullScreenGraphBtn, &QPushButton::clicked, this, &MainWindow::onFullScreenGraph);
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

void MainWindow::onFullScreenGraph() {
    QDialog *graphDialog = new QDialog(this);
    graphDialog->setWindowTitle("Wait-For Graph (Full Screen)");
    QGraphicsView *fullScreenGraphView = new QGraphicsView(graphScene, graphDialog);
    fullScreenGraphView->setRenderHint(QPainter::Antialiasing);
    fullScreenGraphView->setDragMode(QGraphicsView::ScrollHandDrag);
    fullScreenGraphView->setInteractive(true);
    QVBoxLayout *dialogLayout = new QVBoxLayout(graphDialog);
    dialogLayout->addWidget(fullScreenGraphView);
    graphDialog->setLayout(dialogLayout);
    graphDialog->showFullScreen();
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

    double radius = std::max(100.0, 50.0 * transactions.size());
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

    // Build the wait-for graph and track bidirectional edges
    std::map<std::pair<int, int>, bool> hasReverseEdge;
    std::map<int, std::set<int>> waitForGraph;
    for (const auto& T : transactions) {
        if (T.active && !T.waitingFor.empty()) {
            for (int did : T.waitingFor) {
                auto it = lockTable.find(did);
                if (it != lockTable.end() && it->second != -1 && it->second != T.id) {
                    int from = T.id;
                    int to = it->second;
                    waitForGraph[from].insert(to);
                    hasReverseEdge[{from, to}] = waitForGraph[to].count(from) > 0;
                }
            }
        }
    }

    // Draw curved edges with arrows
    for (const auto& pair : waitForGraph) {
        int from = pair.first;
        QPointF fromPos = positions[from];
        for (int to : pair.second) {
            QPointF toPos = positions[to];
            QPointF vec = toPos - fromPos;
            QPointF perp(-vec.y(), vec.x());
            double length = std::sqrt(perp.x()*perp.x() + perp.y()*perp.y());
            if (length > 0) {
                perp /= length;
            }

            // Determine curve direction: above or below based on bidirectional edge
            double offset = 20.0;
            bool hasReverse = hasReverseEdge[{from, to}] || hasReverseEdge[{to, from}];
            if (hasReverse) {
                // If there's a reverse edge, curve one above and one below
                // Use transaction IDs to consistently decide direction
                if (from < to) {
                    // Curve above
                    perp *= offset;
                } else {
                    // Curve below
                    perp *= -offset;
                }
            } else {
                // Single direction, slight curve for clarity
                perp *= offset * 0.5;
            }

            QPointF midpoint = (fromPos + toPos) / 2;
            QPointF controlPoint = midpoint + perp;
            QPainterPath path;
            path.moveTo(fromPos);
            path.quadTo(controlPoint, toPos);

            // Add arrowhead using QVector2D for normalization
            QVector2D vec2D(vec);
            QVector2D normalizedVec = vec2D.normalized();
            QPointF arrowP1 = toPos - (normalizedVec.toPointF() * 10);

            QVector2D perpVec(-vec.y(), vec.x());
            QVector2D normalizedPerp = perpVec.normalized();
            QPointF arrowP2 = arrowP1 + (normalizedPerp.toPointF() * 5);
            QPointF arrowP3 = arrowP1 - (normalizedPerp.toPointF() * 5);

            QPainterPath arrowPath;
            arrowPath.moveTo(toPos);
            arrowPath.lineTo(arrowP2);
            arrowPath.lineTo(arrowP3);
            arrowPath.lineTo(toPos);

            // Add the path and arrowhead to the scene
            graphScene->addPath(path, QPen(Qt::blue, 4));
            graphScene->addPath(arrowPath, QPen(Qt::blue, 4));
        }
    }
}
