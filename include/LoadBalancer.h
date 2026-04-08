#pragma once

#include <vector>
#include "Server.h"

// for load balancing strategies
// abstract base class
class LoadBalancer {
public:
    virtual ~LoadBalancer() = default;

    virtual Server* selectServer(std::vector<Server*>& servers, double currentTime) = 0;
    virtual const char* name() const = 0;
};

class RoundRobinBalancer : public LoadBalancer {
public:
    RoundRobinBalancer();
    Server* selectServer(std::vector<Server*>& servers, double currentTime) override;
    const char* name() const override;

private:
    int lastIndex_;
};

class LeastConnectionsBalancer : public LoadBalancer {
public:
    Server* selectServer(std::vector<Server*>& servers, double currentTime) override;
    const char* name() const override;
};
