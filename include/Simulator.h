#pragma once
#include "SimClock.h"
#include "AutoScaler.h"
#include "LoadBalancer.h"
#include "Request.h"
#include "RequestQueue.h"
#include "ServerCluster.h"
#include <random>
#include <vector>

enum class ArrivalMode {
  Constant,
  Sine,
  Burst,
};

struct SimConfig {
  double tickSize = 1.0;
  double duration = 1000.0;

  int initialServers = 2;
  int minServers = 1;
  int maxServers = 10;

  int scaleUpThresh = 5;
  int scaleDownThresh = 1;
  double cooldown = 20.0;
  double provisionDelay = 0.0;

  // stochastic arrivals, service times
  // req per unit time
  double arrivalRate = 2.0;
  ArrivalMode arrivalMode = ArrivalMode::Constant;
  
  double arrivalPeriod = 200.0;
  // sine mode: amplitude in [0, 1] keeps rate nonnegative
  double arrivalVariation = 0.75;
  
  // burst mode: alternating high and low windows
  double burstOnDuration = 20.0;
  double burstOffDuration = 80.0;
  double burstPeakMultiplier = 4.0;
  double burstLowMultiplier = 0.25;

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

  double totalProvisionedTime = 0.0;
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
  double instantaneousArrivalRate(double t) const;
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
