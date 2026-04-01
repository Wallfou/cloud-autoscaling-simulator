# Cloud Autoscaling Simulator

## Team

This is an individual project by Wa Fan

## Project overview

This repository implements a discrete-time simulation of a simplified cloud-style workload: client requests arrive (or are injected), wait in a shared queue, are assigned to servers by a load balancer, and complete after a modeled service time. An auto-scaler adjusts the number of servers based on queue depth and configured thresholds (including a cooldown to limit churn).

The goal is to study how auto-scaling and load-balancing policies affect latency, throughput, utilization, and cost—without needing real infrastructure.


## Main functionalities

- **Discrete-time simulation** — Configurable tick size and run duration.
- **Server cluster** — Add/remove servers; each server processes assigned requests over time
- **Request queue** — FIFO queue between arrivals and dispatch.
- **Load balancing** — Pluggable strategies via an abstract `LoadBalancer` interface; includes **round-robin** and **least-connections** implementations.
- **Auto-scaling** — Thresholds for scale-up / scale-down, min/max fleet size, and cooldown. 
- **Metrics** — Tracks completed requests, aggregate wait/response time, server uptime, and derived averages and throughput.
- **CLI** — Select balancer and basic simulation parameters from the command line.



## OOP design summary

| Concept | Role in this project |
|--------|----------------------|
| **`Simulator`** | Orchestrates the simulation loop: advances the clock, runs auto-scaling, dispatches the queue through the balancer, updates servers, and aggregates metrics. |
| **`SimConfig` and `SimMetrics`** | Plain data structures for configuration and aggregated statistics (with small helpers on `SimMetrics`). |
| **`LoadBalancer`** | Strategy pattern: `selectServer` and `name` are virtual; `RoundRobinBalancer` and `LeastConnectionsBalancer` supply concrete policies. |
| **`ServerCluster`** | Owns a collection of `Server` instances and supports adding servers for scaling. |
| **`Server`** | Holds optional work, advances processing per tick, exposes utilization/uptime-style data for metrics. |
| **`Request` / `RequestQueue`** | Model units of work and a FIFO queue between arrival and assignment. |
| **`AutoScaler`** | Encapsulates scale-up/scale-down rules and cooldown using cluster + queue state. |
| **`SimClock`** | Encapsulates simulation time and tick duration. |


## Tools and technologies

| Category | Details |
|----------|---------|
| **Language** | C++17 |
| **Build** | `Makefile`, `g++` |
| **Version control** | Github |
| **Development** | VSCode |

**Build and run**

```bash
make          # builds the `simulator` binary
./simulator --balancer rr --duration 1000 --servers 2
make clean    # remove build artifacts
```


## Folder structure

```
cloud-autoscaling-simulator/
├── include/          # Headers (.h) — interfaces and declarations
├── src/              # Implementation (.cpp) and main entry point
├── build/            # Object files (created by `make`; gitignored if listed)
├── Makefile          # Build rules
└── README.md         # This file
```


## Project goals

- Provide a modular simulation core that can swap load-balancing strategies and tune auto-scaling parameters.
- Compare policies using wait time, response time, throughput, utilization, and (eventually) operational cost.
- Support repeatable experiments** under different traffic patterns (e.g., low, bursty, sustained high load) as the implementation matures.
- Deliver a clear codebase and documentation suitable for course or portfolio review.


## GitHub repository purpose

This GitHub repository is the home for source code, revision history, and documentation. It allows instructors, teammates, and reviewers to clone the project, build it with the provided `Makefile`, track changes over time, and inspect design decisions through commits and this README. It also serves as a portfolio artifact demonstrating systems-style C++ and object-oriented design applied to cloud autoscaling concepts.

