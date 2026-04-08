#pragma once

#include <vector>
#include "Server.h"

// this is for implementing a collection of servers
class ServerCluster {
public:
    ServerCluster();
    ~ServerCluster();

    Server* addServer(double readyTime = 0.0);

    // should remove the last idle server, for optimization
    bool removeServer();

    // const and non const versions so i can both read and write to server vector
    std::vector<Server*>& getServers();
    const std::vector<Server*>& getServers() const;

    int size() const;
    int idleCount() const;
    int busyCount() const;
    int acceptingCount(double currentTime) const;

    // same as server update method but for the whole cluster, should return all complete reqs from
    // all the servers in the cluster
    std::vector<Request*> update(double currentTime, double dt);

private:
    std::vector<Server*> servers_;
    int nextId_;
};
