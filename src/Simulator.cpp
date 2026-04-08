#include "../include/Simulator.h"
#include <iostream>


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
              config.maxServers),
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
              << ", servers=" << config_.initialServers << "\n";

    while (clock_.getTime() < config_.duration) {
        step();
        clock_.tick();
    }

    std::cout << "[Simulator] Done. t=" << clock_.getTime()
              << " completed=" << metrics_.completedRequests
              << "/" << metrics_.totalRequests << "\n";
}

void Simulator::generateArrivals(double t, double dt) {
    if (config_.arrivalRate <= 0.0 || dt <= 0.0) return;

    const double lambda = config_.arrivalRate * dt;
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
}

// should loop thru all servers and dispatch requests to available servers
void Simulator::dispatchQueuedRequests() {
    double t = clock_.getTime();
    while (!queue_.empty()) {
        Server* server = balancer_->selectServer(cluster_.getServers());
        if (!server) break;

        Request* req = queue_.dequeue();
        server->assignRequest(req, t);
    }
}

// colelct metrics for all completed requests
void Simulator::collectMetrics(const std::vector<Request*>& completed) {
    for (Request* r : completed) {
        ++metrics_.completedRequests;
        metrics_.totalWaitTime     += r->getWaitTime();
        metrics_.totalResponseTime += r->getResponseTime();
    }
    for (const Server* s : cluster_.getServers()) {
        metrics_.totalUptime += s->getUptime();
    }
}

const SimMetrics& Simulator::getMetrics() const {
    return metrics_;
}

double Simulator::getCurrentTime() const {
    return clock_.getTime();
}
