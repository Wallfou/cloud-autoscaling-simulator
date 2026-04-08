#include "../include/Simulator.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace {

double twoPi() {
    return 2.0 * std::acos(-1.0);
}

double nearestRankPercentile(std::vector<double> values, double p) {
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const size_t n = values.size();
    const size_t k = static_cast<size_t>(std::ceil(p * static_cast<double>(n)));
    const size_t idx = std::min(std::max<size_t>(1, k), n) - 1;
    return values[idx];
}

}


double SimMetrics::avgWaitTime() const {
    if (completedRequests == 0) return 0.0;
    return totalWaitTime / completedRequests;
}

double SimMetrics::avgResponseTime() const {
    if (completedRequests == 0) return 0.0;
    return totalResponseTime / completedRequests;
}

double SimMetrics::throughput(double duration) const {
    if (duration <= 0.0) return 0.0;
    return static_cast<double>(completedRequests) / duration;
}


Simulator::Simulator(const SimConfig& config, LoadBalancer* balancer)
    : config_(config),
      clock_(config.tickSize),
      balancer_(balancer),
      scaler_(config.scaleUpThresh,
              config.scaleDownThresh,
              config.cooldown,
              config.minServers,
              config.maxServers,
              config.provisionDelay),
      nextRequestId_(0),
      rng_([this]() {
        if (config_.randomSeed != 0u) return config_.randomSeed;
        std::random_device rd;
        return rd();
      }()),
      serviceTimeDist_(config_.serviceTimeMin, config_.serviceTimeMax) {
    for (int i = 0; i < config_.initialServers; ++i) {
        cluster_.addServer();
    }
}

Simulator::~Simulator() {
    for (Request* r : allRequests_) {
        delete r;
    }
}

void Simulator::run() {
    std::cout << "[Simulator] Starting with balancer: " << balancer_->name()
              << ", tick=" << config_.tickSize
              << ", duration=" << config_.duration
              << ", servers=" << config_.initialServers
              << ", arrival_mode=";
    switch (config_.arrivalMode) {
        case ArrivalMode::Constant: std::cout << "constant"; break;
        case ArrivalMode::Sine: std::cout << "sine"; break;
        case ArrivalMode::Burst: std::cout << "burst"; break;
    }
    std::cout << "\n";

    while (clock_.getTime() < config_.duration) {
        step();
        clock_.tick();
    }

    finalizeResourceMetrics();

    std::cout << "[Simulator] Done. t=" << clock_.getTime()
              << " completed = " << metrics_.completedRequests
              << "/" << metrics_.totalRequests
              << " servers_final = " << cluster_.size() << "\n";
}

double Simulator::instantaneousArrivalRate(double t) const {
    switch (config_.arrivalMode) {
        case ArrivalMode::Constant:
            return config_.arrivalRate;
        case ArrivalMode::Sine: {
            if (config_.arrivalPeriod <= 0.0) return config_.arrivalRate;
            const double w = twoPi() * t / config_.arrivalPeriod;
            const double lam = config_.arrivalRate * (1.0 + config_.arrivalVariation * std::sin(w));
            return std::max(0.0, lam);
        }
        case ArrivalMode::Burst: {
            const double cycle = config_.burstOnDuration + config_.burstOffDuration;
            if (cycle <= 0.0) return config_.arrivalRate;
            double pos = std::fmod(t, cycle);
            if (pos < 0.0) pos += cycle;
            const bool onBurst = pos < config_.burstOnDuration;
            const double mul =
                onBurst ? config_.burstPeakMultiplier : config_.burstLowMultiplier;
            return std::max(0.0, config_.arrivalRate * mul);
        }
    }
    return config_.arrivalRate;
}

void Simulator::generateArrivals(double t, double dt) {
    if (dt <= 0.0) return;

    const double lambdaInstant = instantaneousArrivalRate(t);
    if (lambdaInstant <= 0.0) return;

    const double lambda = lambdaInstant * dt;
    std::poisson_distribution<int> numArrivals(lambda);
    const int n = numArrivals(rng_);

    for (int i = 0; i < n; ++i) {
        double serviceTime = serviceTimeDist_(rng_);
        if (serviceTime <= 0.0) serviceTime = config_.serviceTimeMin;

        Request* req = new Request(nextRequestId_++, t, serviceTime);
        allRequests_.push_back(req);
        queue_.enqueue(req);
        ++metrics_.totalRequests;
    }
}

void Simulator::step() {
    double t  = clock_.getTime();
    double dt = clock_.getTickSize();

    generateArrivals(t, dt);

    scaler_.evaluate(cluster_, queue_, t);

    dispatchQueuedRequests();

    std::vector<Request*> completed = cluster_.update(t + dt, dt);
    collectMetrics(completed);

    metrics_.totalProvisionedTime += static_cast<double>(cluster_.size()) * dt;
}

// should loop thru all servers and dispatch requests to available servers
void Simulator::dispatchQueuedRequests() {
    double t = clock_.getTime();
    while (!queue_.empty()) {
        Server* server = balancer_->selectServer(cluster_.getServers(), t);
        if (!server) break;

        Request* req = queue_.dequeue();
        server->assignRequest(req, t);
    }
}

void Simulator::collectMetrics(const std::vector<Request*>& completed) {
    for (Request* r : completed) {
        ++metrics_.completedRequests;
        metrics_.totalWaitTime     += r->getWaitTime();
        metrics_.totalResponseTime += r->getResponseTime();
        completedWaitSamples_.push_back(r->getWaitTime());
        completedResponseSamples_.push_back(r->getResponseTime());
    }
}

void Simulator::finalizeResourceMetrics() {
    double busy = 0.0;
    for (const Server* s : cluster_.getServers()) {
        busy += s->getUptime();
    }
    metrics_.totalBusyTime = busy;

    metrics_.p95WaitTime = nearestRankPercentile(completedWaitSamples_, 0.95);
    metrics_.p95ResponseTime = nearestRankPercentile(completedResponseSamples_, 0.95);
}

const SimMetrics& Simulator::getMetrics() const {
    return metrics_;
}

double Simulator::getCurrentTime() const {
    return clock_.getTime();
}
