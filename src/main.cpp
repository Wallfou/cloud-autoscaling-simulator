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
              << "  --arrival-mode <m>   constant | sine | burst (default: constant)\n"
              << "  --arrival-rate <λ>   Baseline Poisson rate (requests / time; default: 2.0)\n"
              << "  --arrival-period <t> Sine mode: cycle length in time units (default: 200)\n"
              << "  --arrival-variation <a> Sine mode: amplitude a in λ*(1+a*sin(...)), use 0..1 (default: 0.75)\n"
              << "  --burst-on <t>       Burst mode: high-rate window length (default: 20)\n"
              << "  --burst-off <t>      Burst mode: low-rate window length (default: 80)\n"
              << "  --burst-peak-mul <m> Burst mode: rate multiplier in on window (default: 4)\n"
              << "  --burst-low-mul <m>  Burst mode: rate multiplier in off window (default: 0.25)\n"
              << "  --service-min <t>    Min service time per request (default: 1.0)\n"
              << "  --service-max <t>    Max service time per request (default: 5.0)\n"
              << "  --seed <n>           RNG seed (default: 42; use 0 for non-deterministic)\n"
              << "  --scale-up <n>       Scale out when queue length >= n (default: 5)\n"
              << "  --scale-down <n>     Scale in when queue length <= n (default: 1)\n"
              << "  --cooldown <t>       Min time between scaling actions (default: 20)\n"
              << "  --min-servers <n>    Minimum cluster size (default: 1)\n"
              << "  --max-servers <n>    Maximum cluster size (default: 10)\n"
              << "  --provision-delay <t> Seconds before a new instance accepts traffic (default: 0)\n"
              << "  --cost-rate <c>      Cost per server-time unit of provisioned capacity (default: 0 = off)\n"
              << "  --help               Show this message\n";
}

int main(int argc, char* argv[]) {
    SimConfig config;
    std::string balancerType = "rr";
    std::string arrivalModeStr = "constant";

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
        } else if (arg == "--arrival-mode" && i + 1 < argc) {
            arrivalModeStr = argv[++i];
        } else if (arg == "--arrival-rate" && i + 1 < argc) {
            config.arrivalRate = std::stod(argv[++i]);
        } else if (arg == "--arrival-period" && i + 1 < argc) {
            config.arrivalPeriod = std::stod(argv[++i]);
        } else if (arg == "--arrival-variation" && i + 1 < argc) {
            config.arrivalVariation = std::stod(argv[++i]);
        } else if (arg == "--burst-on" && i + 1 < argc) {
            config.burstOnDuration = std::stod(argv[++i]);
        } else if (arg == "--burst-off" && i + 1 < argc) {
            config.burstOffDuration = std::stod(argv[++i]);
        } else if (arg == "--burst-peak-mul" && i + 1 < argc) {
            config.burstPeakMultiplier = std::stod(argv[++i]);
        } else if (arg == "--burst-low-mul" && i + 1 < argc) {
            config.burstLowMultiplier = std::stod(argv[++i]);
        } else if (arg == "--service-min" && i + 1 < argc) {
            config.serviceTimeMin = std::stod(argv[++i]);
        } else if (arg == "--service-max" && i + 1 < argc) {
            config.serviceTimeMax = std::stod(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            config.randomSeed = static_cast<unsigned>(std::stoul(argv[++i]));
        } else if (arg == "--scale-up" && i + 1 < argc) {
            config.scaleUpThresh = std::stoi(argv[++i]);
        } else if (arg == "--scale-down" && i + 1 < argc) {
            config.scaleDownThresh = std::stoi(argv[++i]);
        } else if (arg == "--cooldown" && i + 1 < argc) {
            config.cooldown = std::stod(argv[++i]);
        } else if (arg == "--min-servers" && i + 1 < argc) {
            config.minServers = std::stoi(argv[++i]);
        } else if (arg == "--max-servers" && i + 1 < argc) {
            config.maxServers = std::stoi(argv[++i]);
        } else if (arg == "--provision-delay" && i + 1 < argc) {
            config.provisionDelay = std::stod(argv[++i]);
        } else if (arg == "--cost-rate" && i + 1 < argc) {
            config.costPerProvisionedTime = std::stod(argv[++i]);
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

    if (config.minServers < 1 || config.maxServers < config.minServers) {
        std::cerr << "Invalid server bounds: need 1 <= min servers <= max\n";
        delete balancer;
        return 1;
    }
    if (config.initialServers < config.minServers || config.initialServers > config.maxServers) {
        std::cerr << "Initial servers must be between min and max servers\n";
        delete balancer;
        return 1;
    }
    if (config.provisionDelay < 0.0) {
        std::cerr << "Provision delay must be >= 0\n";
        delete balancer;
        return 1;
    }
    if (config.costPerProvisionedTime < 0.0) {
        std::cerr << "Cost rate must be >= 0\n";
        delete balancer;
        return 1;
    }

    if (arrivalModeStr == "constant") {
        config.arrivalMode = ArrivalMode::Constant;
    } else if (arrivalModeStr == "sine") {
        config.arrivalMode = ArrivalMode::Sine;
    } else if (arrivalModeStr == "burst") {
        config.arrivalMode = ArrivalMode::Burst;
    } else {
        std::cerr << "Unknown arrival mode: " << arrivalModeStr << "\n";
        delete balancer;
        return 1;
    }

    if (config.arrivalMode == ArrivalMode::Sine && config.arrivalPeriod <= 0.0) {
        std::cerr << "Sine mode requires --arrival-period > 0\n";
        delete balancer;
        return 1;
    }
    if (config.arrivalMode == ArrivalMode::Burst) {
        if (config.burstOnDuration < 0.0 || config.burstOffDuration < 0.0) {
            std::cerr << "Burst mode: --burst-on and --burst-off must be >= 0\n";
            delete balancer;
            return 1;
        }
        if (config.burstOnDuration + config.burstOffDuration <= 0.0) {
            std::cerr << "Burst mode: sum of on + off durations must be positive\n";
            delete balancer;
            return 1;
        }
        if (config.burstPeakMultiplier < 0.0 || config.burstLowMultiplier < 0.0) {
            std::cerr << "Burst multpliers must be >= 0\n";
            delete balancer;
            return 1;
        }
    }

    Simulator sim(config, balancer);
    sim.run();

    const SimMetrics& m = sim.getMetrics();

    std::cout << "\n=== Sample run ===\n"
              << "Balancer:           " << balancer->name() << "\n"
              << "Arrival mode:       " << arrivalModeStr << "\n"
              << "Baseline rate:      " << config.arrivalRate << " req/time\n";
    if (config.arrivalMode == ArrivalMode::Sine) {
        std::cout << "Sine period:        " << config.arrivalPeriod
                  << ", variation: " << config.arrivalVariation << "\n";
    } else if (config.arrivalMode == ArrivalMode::Burst) {
        std::cout << "Burst on/off:       " << config.burstOnDuration << " / "
                  << config.burstOffDuration << ", peak/low mul: "
                  << config.burstPeakMultiplier << " / " << config.burstLowMultiplier << "\n";
    }
    std::cout << "Service time:       U[" << config.serviceTimeMin << ", " << config.serviceTimeMax << "]\n"
              << "Seed:               "
              << (config.randomSeed == 0u ? std::string("(random)") : std::to_string(config.randomSeed))
              << "\n"
              << "Autoscale:          queue >= " << config.scaleUpThresh << " out, queue <= "
              << config.scaleDownThresh << " in, cooldown=" << config.cooldown << "\n"
              << "Server bounds:      [" << config.minServers << ", " << config.maxServers << "]\n"
              << "Provision delay:    " << config.provisionDelay << "\n"
              << "\n=== Results ===\n"
              << "Total requests:     " << m.totalRequests << "\n"
              << "Completed requests: " << m.completedRequests << "\n"
              << "Avg wait time:      " << m.avgWaitTime() << "\n"
              << "Avg response time:  " << m.avgResponseTime() << "\n"
              << "p95 wait time:      " << m.p95WaitTime << "\n"
              << "p95 response time:  " << m.p95ResponseTime << "\n"
              << "Throughput:         " << m.throughput(config.duration) << " req/time unit\n"
              << "Provisioned time:   " << m.totalProvisionedTime << "\n"
              << "Total busy time:    " << m.totalBusyTime << " (sum of server processing time)\n"
              << "Final servers:      " << m.finalServers << "\n";
    if (m.totalProvisionedTime > 0.0) {
        std::cout << "Utilization (busy/provisioned): " << (m.totalBusyTime / m.totalProvisionedTime) << "\n";
    }
    if (config.costPerProvisionedTime > 0.0) {
        const double estCost = m.totalProvisionedTime * config.costPerProvisionedTime;
        std::cout << "Est. provision cost: " << estCost
                  << " (= provisioned_time * cost_rate=" << config.costPerProvisionedTime << ")\n";
    }

    delete balancer;
    return 0;
}
