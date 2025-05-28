#include "LockManager.h"
#include <algorithm>
#include <sstream>

LockManager::LockManager() : preventionEnabled(false), nextTid(0), nextDid(0) {}

int LockManager::addTransaction() {
    transactions.push_back(Transaction(nextTid));
    return nextTid++;
}

int LockManager::addDataItem() {
    dataItems.push_back(nextDid);
    lockTable[nextDid] = -1; // Initially free
    return nextDid++;
}

bool LockManager::requestLock(int tid, int did, std::string& log) {
    if (tid >= static_cast<int>(transactions.size()) || !transactions[tid].active) {
        log = "Invalid or inactive transaction T" + std::to_string(tid);
        return false;
    }
    if (std::find(dataItems.begin(), dataItems.end(), did) == dataItems.end()) {
        log = "Data item D" + std::to_string(did) + " does not exist";
        return false;
    }

    auto& T = transactions[tid];
    if (preventionEnabled && !T.heldLocks.empty()) {
        int maxHeld = *std::max_element(T.heldLocks.begin(), T.heldLocks.end());
        if (did <= maxHeld) {
            log = "Request denied: T" + std::to_string(tid) + " cannot request D" +
                  std::to_string(did) + " (holds D" + std::to_string(maxHeld) + ")";
            return false;
        }
    }

    auto it = lockTable.find(did);
    if (it->second == -1) { // Lock is free
        lockTable[did] = tid;
        T.heldLocks.insert(did);
        log = "T" + std::to_string(tid) + " acquired lock on D" + std::to_string(did);
        return true;
    } else {
        T.waitingFor.insert(did);
        log = "T" + std::to_string(tid) + " waiting for D" + std::to_string(did) +
              " held by T" + std::to_string(it->second);
        return false;
    }
}

void LockManager::releaseLock(int tid, int did, std::string& log) {
    if (tid >= static_cast<int>(transactions.size()) || !transactions[tid].active) {
        log = "Invalid or inactive transaction T" + std::to_string(tid);
        return;
    }
    auto& T = transactions[tid];
    if (T.heldLocks.find(did) != T.heldLocks.end()) {
        T.heldLocks.erase(did);
        lockTable[did] = -1;
        log = "T" + std::to_string(tid) + " released lock on D" + std::to_string(did);
    } else {
        log = "T" + std::to_string(tid) + " does not hold D" + std::to_string(did);
    }
}

bool LockManager::detectDeadlock(std::string& log, std::vector<int>& cycle) {
    cycle = findCycle();
    if (!cycle.empty()) {
        std::stringstream ss;
        ss << "Deadlock detected: ";
        for (size_t i = 0; i < cycle.size(); ++i) {
            ss << "T" << cycle[i];
            if (i < cycle.size() - 1) ss << " -> ";
        }
        log = ss.str();
        return true;
    }
    log = "No deadlock detected";
    return false;
}

void LockManager::recover(const std::vector<int>& cycle, std::string& log) {
    if (cycle.empty()) {
        log = "No deadlock to recover from";
        return;
    }
    int tidToTerminate = *std::max_element(cycle.begin(), cycle.end());
    auto& T = transactions[tidToTerminate];
    T.active = false;
    for (int did : T.heldLocks) {
        lockTable[did] = -1;
    }
    T.heldLocks.clear();
    T.waitingFor.clear();
    log = "Terminated T" + std::to_string(tidToTerminate) + " to resolve deadlock";
}

std::vector<int> LockManager::findCycle() {
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

    std::set<int> visited;
    std::vector<int> path, cycle;
    for (const auto& pair : waitForGraph) {
        int tid = pair.first;
        if (visited.find(tid) == visited.end()) {
            if (dfsCycle(tid, waitForGraph, visited, path, cycle)) {
                return cycle;
            }
        }
    }
    return {};
}

bool LockManager::dfsCycle(int tid, const std::map<int, std::set<int>>& waitForGraph,
                           std::set<int>& visited, std::vector<int>& path, std::vector<int>& cycle) {
    visited.insert(tid);
    path.push_back(tid);
    auto it = waitForGraph.find(tid);
    if (it != waitForGraph.end()) {
        for (int neighbor : it->second) {
            if (std::find(path.begin(), path.end(), neighbor) != path.end()) {
                auto start = std::find(path.begin(), path.end(), neighbor);
                cycle.assign(start, path.end());
                cycle.push_back(neighbor);
                return true;
            }
            if (visited.find(neighbor) == visited.end()) {
                if (dfsCycle(neighbor, waitForGraph, visited, path, cycle)) {
                    return true;
                }
            }
        }
    }
    path.pop_back();
    return false;
}
