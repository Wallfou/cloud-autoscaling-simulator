#include "../include/ServerCluster.h"

ServerCluster::ServerCluster() : nextId_(0) {}
ServerCluster::~ServerCluster() {
    for (Server* s : servers_) {
        delete s;
    }
}

Server* ServerCluster::addServer(double readyTime) {
    Server* s = new Server(nextId_++, readyTime);
    servers_.push_back(s);
    return s;
}

bool ServerCluster::removeServer() {
    int minSize = 1;
    if (servers_.size() <= minSize) return false;

    for (int i = servers_.size() - 1; i >= 0; i--) {
        if (!servers_[i]->isBusy()) {
            delete servers_[i];
            servers_.erase(servers_.begin() + i);
            return true;
        }
    }
    return false;
}

std::vector<Request*> ServerCluster::update(double currentTime, double dt) {
    std::vector<Request*> completed;
    for (Server* s : servers_) {
        s->updateUptime(dt);
        Request* done = s->update(currentTime, dt);
        if (done) completed.push_back(done);
    }
    return completed;
}

std::vector<Server*>& ServerCluster::getServers() {
    return servers_;
}

const std::vector<Server*>& ServerCluster::getServers() const {
    return servers_;
}

int ServerCluster::size() const {
    return servers_.size();
}

int ServerCluster::idleCount() const {
    int count = 0;
    for (const Server* s : servers_) {
        if (!s->isBusy()) count++;
    }
    return count;
}

int ServerCluster::busyCount() const {
    return servers_.size() - idleCount();
}

int ServerCluster::acceptingCount(double currentTime) const {
    int count = 0;
    for (const Server* s : servers_) {
        if (s->canAcceptWork(currentTime)) ++count;
    }
    return count;
}
