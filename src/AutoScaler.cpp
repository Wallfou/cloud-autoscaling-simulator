#include "../include/AutoScaler.h"

AutoScaler::AutoScaler(int scaleUpQueueThreshold, int scaleDownQueueThreshold, double cooldownPeriod,
                       int minServers, int maxServers)
    : scaleUpQueueThreshold_(scaleUpQueueThreshold),
      scaleDownQueueThreshold_(scaleDownQueueThreshold),
      cooldownPeriod_(cooldownPeriod),
      minServers_(minServers),
      maxServers_(maxServers),
      lastScaleTime_(-1e9) {}

void AutoScaler::evaluate(ServerCluster& cluster, const RequestQueue& queue, double currentTime) {
    // todo 
}

int AutoScaler::getScaleUpThreshold() const {
    return scaleUpQueueThreshold_;
}

int AutoScaler::getScaleDownThreshold() const {
    return scaleDownQueueThreshold_;
}

double AutoScaler::getCooldownPeriod() const {
    return cooldownPeriod_;
}
