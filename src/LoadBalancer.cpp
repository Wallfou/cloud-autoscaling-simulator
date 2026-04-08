#include "../include/LoadBalancer.h"

RoundRobinBalancer::RoundRobinBalancer() : lastIndex_(-1) {}

Server* RoundRobinBalancer::selectServer(std::vector<Server*>& servers) {
    if (servers.empty()) return nullptr;

    const int n = static_cast<int>(servers.size());
    for (int step = 0; step < n; ++step) {
        const int idx = (lastIndex_ + 1 + step) % n;
        if (!servers[idx]->isBusy()) {
            lastIndex_ = idx;
            return servers[idx];
        }
    }
    return nullptr;
}

const char* RoundRobinBalancer::name() const {
    return "RoundRobin";
}

Server* LeastConnectionsBalancer::selectServer(std::vector<Server*>& servers) {
    Server* best = nullptr;
    for (Server* s : servers) {
        if (s->isBusy()) continue;
        if (!best || s->getId() < best->getId()) best = s;
    }
    return best;
}

const char* LeastConnectionsBalancer::name() const {
    return "LeastConnections";
}
