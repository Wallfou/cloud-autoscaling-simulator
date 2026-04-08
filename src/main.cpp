#include <iostream>
#include <string>

#include "../include/Simulator.h"
#include "../include/LoadBalancer.h"

static void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "  --balancer <rr|lc>   Load balancer: round-robin or least-connections (default: rr)\n"
              << "  --tick <size>        Simulation tick size (default: 1.0)\n"
              << "  --duration <t>       Simulation duration  (default: 1000.0)\n"
              << "  --servers <n>        Initial server count (default: 2)\n"
              << "  --arrival-rate <λ>   Poisson arrival rate (requests / time; default: 2.0)\n"
              << "  --service-min <t>    Min service time per request (default: 1.0)\n"
              << "  --service-max <t>    Max service time per request (default: 5.0)\n"
              << "  --seed <n>           RNG seed (default: 42; use 0 for non-deterministic)\n"
              << "  --help               Show this message\n";
}

int main(int argc, char* argv[]) {
    SimConfig config;
    std::string balancerType = "rr";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--balancer" && i + 1 < argc) {
            balancerType = argv[++i];
        } else if (arg == "--tick" && i + 1 < argc) {
            config.tickSize = std::stod(argv[++i]);
        } else if (arg == "--duration" && i + 1 < argc) {
            config.duration = std::stod(argv[++i]);
        } else if (arg == "--servers" && i + 1 < argc) {
            config.initialServers = std::stoi(argv[++i]);
        } else if (arg == "--arrival-rate" && i + 1 < argc) {
            config.arrivalRate = std::stod(argv[++i]);
        } else if (arg == "--service-min" && i + 1 < argc) {
            config.serviceTimeMin = std::stod(argv[++i]);
        } else if (arg == "--service-max" && i + 1 < argc) {
            config.serviceTimeMax = std::stod(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            config.randomSeed = static_cast<unsigned>(std::stoul(argv[++i]));
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    LoadBalancer* balancer = nullptr;
    if (balancerType == "rr") {
        balancer = new RoundRobinBalancer();
    } else if (balancerType == "lc") {
        balancer = new LeastConnectionsBalancer();
    } else {
        std::cerr << "Unknown balancer type: " << balancerType << "\n";
        return 1;
    }

    if (config.serviceTimeMin <= 0.0 || config.serviceTimeMax < config.serviceTimeMin) {
        std::cerr << "Invalid service time range: need 0 < service min <= max\n";
        delete balancer;
        return 1;
    }

    Simulator sim(config, balancer);
    sim.run();

    const SimMetrics& m = sim.getMetrics();
    std::cout << "\n=== Sample run ===\n"
              << "Balancer:           " << balancer->name() << "\n"
              << "Arrival rate:       " << config.arrivalRate << " req/time\n"
              << "Service time:       U[" << config.serviceTimeMin << ", " << config.serviceTimeMax << "]\n"
              << "Seed:               " << (config.randomSeed == 0u ? std::string("(random)") : std::to_string(config.randomSeed)) << "\n"
              << "\n=== Results ===\n"
              << "Total requests:     " << m.totalRequests      << "\n"
              << "Completed requests: " << m.completedRequests   << "\n"
              << "Avg wait time:      " << m.avgWaitTime()        << "\n"
              << "Avg response time:  " << m.avgResponseTime()    << "\n"
              << "Throughput:         " << m.throughput(config.duration) << " req/time unit\n";

    delete balancer;
    return 0;
}
