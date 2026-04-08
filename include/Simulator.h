#pragma once
#include "SimClock.h"
#include "AutoScaler.h"
#include "LoadBalancer.h"
#include "Request.h"
#include "RequestQueue.h"
#include "ServerCluster.h"
#include <random>
#include <vector>

struct SimConfig {
  double tickSize = 1.0;
  double duration = 1000.0;

  int initialServers = 2;
  int minServers = 1;
  int maxServers = 10;

  int scaleUpThresh = 5;
  int scaleDownThresh = 1;
  double cooldown = 20.0;

  // stochastic arrivals, service times
  // req per unit time
  double arrivalRate = 2.0;
  double serviceTimeMin = 1.0;
  double serviceTimeMax = 5.0;
  // cahnge to 0 for non-deterministic seed 
  unsigned randomSeed = 42;
};

struct SimMetrics {
  int totalRequests = 0;
  int completedRequests = 0;

  double totalWaitTime = 0.0;
  double totalResponseTime = 0.0;

  /// Sum over ticks of (live server count × dt). Used for operational cost (instance-time).
  double totalProvisionedTime = 0.0;
  /// Sum of per-server busy time at end of run (processing time).
  double totalBusyTime = 0.0;

  double avgWaitTime() const;
  double avgResponseTime() const;
  double throughput(double duration) const;
};

class Simulator {
public:
  Simulator(const SimConfig &config, LoadBalancer *balancer);
  ~Simulator();

  void run();
  const SimMetrics &getMetrics() const;
  double getCurrentTime() const;

private:
  void step();
  void generateArrivals(double t, double dt);
  void dispatchQueuedRequests();
  void collectMetrics(const std::vector<Request*> &completed);
  void finalizeResourceMetrics();

  SimConfig config_;
  SimClock clock_;
  RequestQueue queue_;
  ServerCluster cluster_;
  LoadBalancer *balancer_;
  AutoScaler scaler_;
  SimMetrics metrics_;
  int nextRequestId_;
  std::vector<Request *> allRequests_;

  std::mt19937 rng_;
  std::uniform_real_distribution<double> serviceTimeDist_;
};
