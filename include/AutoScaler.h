#pragma once
#include "RequestQueue.h"
#include "ServerCluster.h"

// should eval cluster state each tick and manage servers if needed
class AutoScaler {
public:
    AutoScaler(int scaleUpQueueThreshold,
               int scaleDownQueueThreshold,
               double cooldownPeriod,
               int minServers,
               int maxServers,
               double provisionDelay);

    void evaluate(ServerCluster& cluster, const RequestQueue& queue, double currentTime);

    int getScaleUpThreshold() const;
    int getScaleDownThreshold() const;
    double getCooldownPeriod() const;

private:
    int scaleUpQueueThreshold_;
    int scaleDownQueueThreshold_;
    double cooldownPeriod_;

    int minServers_;
    int maxServers_;
    double provisionDelay_;

    double lastScaleTime_;
};
