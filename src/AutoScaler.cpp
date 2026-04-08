#include "../include/AutoScaler.h"

AutoScaler::AutoScaler(int scaleUpQueueThreshold, int scaleDownQueueThreshold, double cooldownPeriod,
                       int minServers, int maxServers, double provisionDelay)
    : scaleUpQueueThreshold_(scaleUpQueueThreshold),
      scaleDownQueueThreshold_(scaleDownQueueThreshold),
      cooldownPeriod_(cooldownPeriod),
      minServers_(minServers),
      maxServers_(maxServers),
      provisionDelay_(provisionDelay),
      lastScaleTime_(-1e9) {}

void AutoScaler::evaluate(ServerCluster& cluster, const RequestQueue& queue, double currentTime) {
    if (cooldownPeriod_ > 0.0 && (currentTime - lastScaleTime_) < cooldownPeriod_) {
        return;
    }

    const int q = queue.size();
    const int n = cluster.size();
    const bool noCapacity = (n > 0 && cluster.acceptingCount(currentTime) == 0);
    const bool saturated = noCapacity && q > 0;

    if ((q >= scaleUpQueueThreshold_ || saturated) && n < maxServers_) {
        cluster.addServer(currentTime + provisionDelay_);
        lastScaleTime_ = currentTime;
        return;
    }

    if (q <= scaleDownQueueThreshold_ && n > minServers_ && cluster.idleCount() > 0) {
        if (cluster.removeServer()) {
            lastScaleTime_ = currentTime;
        }
    }
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
