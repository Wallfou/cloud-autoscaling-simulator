#pragma once

#include "Request.h"

// representing a server that process requests
class Server {
public:
    explicit Server(int id, double readyTime = 0.0);

    bool isBusy() const;
    bool canAcceptWork(double currentTime) const;
    int getActiveConnections() const;
    int getId() const;
    double getUptime() const;

    // keep trakc of this server uptime
    void updateUptime(double dt);

    // assign a req to this server and record start time
    void assignRequest(Request* request, double currentTime);

    // updates server by delta time and returns completed requests if any
    Request* update(double currentTime, double dt);

    int getNumberOfRequestsProcessed() const;

private:
    int id_;
    double readyTime_;
    bool busy_;
    Request* currentRequest_;
    double currentRequestRemainingTime_;
    double uptime_;
    int requestsProcessed_;
};
