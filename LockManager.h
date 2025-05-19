#ifndef LOCKMANAGER_H
#define LOCKMANAGER_H

#include <vector>
#include <set>
#include <map>
#include <string>

struct Transaction {
    int id;
    std::set<int> heldLocks;
    std::set<int> waitingFor;
    bool active;
    Transaction(int tid) : id(tid), active(true) {}
};

struct DataItem {
    int id;
    DataItem(int did) : id(did) {}
};

class LockManager {
private:
    std::vector<Transaction> transactions;
    std::vector<int> dataItems;
    std::map<int, int> lockTable; // did -> tid, -1 if free
    bool preventionEnabled;
    int nextTid;
    int nextDid;

    std::vector<int> findCycle();
    bool dfsCycle(int tid, const std::map<int, std::set<int>>& waitForGraph,
                  std::set<int>& visited, std::vector<int>& path, std::vector<int>& cycle);

public:
    LockManager();
    int addTransaction();
    int addDataItem();
    bool requestLock(int tid, int did, std::string& log);
    void releaseLock(int tid, int did, std::string& log);
    bool detectDeadlock(std::string& log, std::vector<int>& cycle);
    void recover(const std::vector<int>& cycle, std::string& log);
    void setPreventionEnabled(bool enabled) { preventionEnabled = enabled; }
    const std::vector<Transaction>& getTransactions() const { return transactions; }
    const std::vector<int>& getDataItems() const { return dataItems; }
    const std::map<int, int>& getLockTable() const { return lockTable; }
};

#endif // LOCKMANAGER_H
