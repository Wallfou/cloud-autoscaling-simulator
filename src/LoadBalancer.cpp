#include "../include/LoadBalancer.h"
#include <limits>

RoundRobinBalancer::RoundRobinBalancer() : lastIndex_(-1) {}

Server* RoundRobinBalancer::selectServer(std::vector<Server*>& servers, double currentTime) {
    if (servers.empty()) return nullptr;

    const int n = static_cast<int>(servers.size());
    for (int step = 0; step < n; ++step) {
        const int idx = (lastIndex_ + 1 + step) % n;
        if (servers[idx]->canAcceptWork(currentTime)) {
            lastIndex_ = idx;
            return servers[idx];
        }
    }
    return nullptr;
}

const char* RoundRobinBalancer::name() const {
    return "RoundRobin";
}

Server* LeastConnectionsBalancer::selectServer(std::vector<Server*>& servers, double currentTime) {
    if (servers.empty()) return nullptr;

    Server* best = nullptr;
    int minConn = std::numeric_limits<int>::max();
    int bestId = std::numeric_limits<int>::max();

    for (Server* s : servers) {
        if (!s->canAcceptWork(currentTime)) continue;
        const int c = s->getActiveConnections();
        if (c < minConn || (c == minConn && s->getId() < bestId)) {
            minConn = c;
            bestId = s->getId();
            best = s;
        }
    }

    if (!best || minConn > 0) return nullptr;
    return best;
}

const char* LeastConnectionsBalancer::name() const {
    return "LeastConnections";
}
