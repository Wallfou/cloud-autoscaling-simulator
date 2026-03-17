#include "../include/LoadBalancer.h"

RoundRobinBalancer::RoundRobinBalancer() : lastIndex_(-1) {}

Server* RoundRobinBalancer::selectServer(std::vector<Server*>& servers) {
    // todo
    return nullptr;
}

const char* RoundRobinBalancer::name() const {
    return "RoundRobin";
}

Server* LeastConnectionsBalancer::selectServer(std::vector<Server*>& servers) {
    // todo
    return nullptr;
}

const char* LeastConnectionsBalancer::name() const {
    return "LeastConnections";
}
