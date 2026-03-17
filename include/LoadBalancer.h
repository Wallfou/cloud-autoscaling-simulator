#pragma once

#include <vector>
#include "Server.h"

// for load balancing strategies
// abstract base class
class LoadBalancer {
public:
    virtual ~LoadBalancer() = default;

    virtual Server* selectServer(std::vector<Server*>& servers) = 0;
    virtual const char* name() const = 0;
};

// these aren't implemented yet : 

class RoundRobinBalancer : public LoadBalancer {
public:
    RoundRobinBalancer();
    Server* selectServer(std::vector<Server*>& servers) override;
    const char* name() const override;

private:
    int lastIndex_;
};

class LeastConnectionsBalancer : public LoadBalancer {
public:
    Server* selectServer(std::vector<Server*>& servers) override;
    const char* name() const override;
};
