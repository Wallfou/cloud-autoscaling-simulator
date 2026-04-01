# Cloud Autoscaling Simulator

## Team

This is an individual project by Wa Fan

## Project overview

This repo is a discrete-time simulation of a simplified cloud workload. Client requests come in, sit in a shared queue, get handed off to servers by a load balancer, and finish after some modeled service time. An auto-scaler watches the queue and adjusts the number of servers based on thresholds (with a cooldown so it doesn't thrash).

The point is to study how different auto-scaling and load-balancing setups affect things like latency, throughput, utilization, and cost — without needing actual infrastructure.

More detail on the architecture and milestones is in `Iteration #3.pdf` and `Project Scope & Timeline.pdf`.

## What it does

- Runs in fixed time steps — you set tick size and total duration.
- Keeps a cluster of servers (grows/shrinks dynamically) and a FIFO request queue.
- Two balancer modes available from CLI: round-robin (`rr`) and least-connections (`lc`).
- Auto-scaling is driven by configurable queue thresholds, plus min/max server counts.
- Collects stats at the end: completed requests, avg wait/response time, throughput, server uptime (for cost analysis down the line).

## Class interaction and responsibility

Each type has one clear job. **`Simulator`** runs the loop. **`SimClock`** tracks time. **`Request`** and **`RequestQueue`** model the backlog. **`Server`** and **`ServerCluster`** model compute resources. **`LoadBalancer`** (plus its concrete subclasses) decides where work goes. **`AutoScaler`** resizes the cluster. **`SimConfig`** / **`SimMetrics`** hold settings and results. The whole thing is set up so you can swap policies and tweak parameters without rewriting the engine.

### Summary table

| Class name | Responsibility | Interacts with | Reason |
|------------|----------------|----------------|--------|
| **`SimConfig`** | Stores run-time parameters (tick size, duration, server bounds, scaling thresholds, cooldown). | Passed into `Simulator`, `SimClock`, `AutoScaler`. | Keeps tunable settings in one place so experiments just change config, not scattered constants. |
| **`SimMetrics`** | Accumulates totals, exposes averages and throughput for completed work. | Filled by `Simulator` using `Request` and `Server` data; `main` reads it for output. | Keeps measurement separate from simulation logic — makes reporting and comparison cleaner. |
| **`SimClock`** | Tracks simulation time and tick length; steps time forward. | `Simulator` calls it every tick; time gets passed into server updates and scaling. | Everything needs to agree on "now" and step size (see Iteration #3 clock spec). |
| **`Request`** | One unit of work — has an ID, arrival time, service duration, and timing fields for wait/response. | `Simulator` creates them; they sit in `RequestQueue` and get processed by `Server`; metrics read the completion fields. | Can't measure latency or compare runs without it. |
| **`RequestQueue`** | FIFO buffer between arrival and dispatch; exposes its length for scaling decisions. | `Simulator` enqueues/dequeues; `AutoScaler` checks the size; `LoadBalancer` indirectly consumes via dispatch. | Models backlog and gives the auto-scaler a clear signal (queue length thresholds). |
| **`Server`** | One machine: busy or idle, processes one job at a time, tracks per-tick uptime. | Gets `Request` from `Simulator`; gets picked by `LoadBalancer`; lives inside `ServerCluster`. | Wraps up processing logic and utilization/uptime data used for cost and metrics. |
| **`ServerCluster`** | Owns a dynamic set of `Server` objects; handles add/remove and cluster-wide `update`. | `Simulator`, `LoadBalancer` (server list), `AutoScaler` (add/remove), individual `Server::update`. | Represents the elastic fleet — what the balancer targets and the scaler resizes. |
| **`LoadBalancer`** (ABC) | Strategy interface: pick an idle server for the next request, or return `nullptr`. | Implementations get `Server` pointers from `ServerCluster`; called by `Simulator::dispatchQueuedRequests`. | Strategy pattern — same `Simulator` code, different policies (Iteration #3 + Project Scope). |
| **`RoundRobinBalancer`** | Cycles through servers in order. | `Server` list from cluster; used only through the `LoadBalancer` pointer. | Baseline policy that spreads work evenly over time. |
| **`LeastConnectionsBalancer`** | Picks the server with the fewest processed requests (rough proxy for load). | Same interface-level interaction as round-robin. | Gives us a second strategy to compare against round-robin under identical conditions. |
| **`AutoScaler`** | Each tick, checks queue length against thresholds and decides whether to add/remove a server, respecting cooldown and min/max bounds. | `ServerCluster`, `RequestQueue` (read-only), current time from the sim step. | Keeps autoscaling policy self-contained so the main loop stays simple (Iteration #3 item 6). |
| **`Simulator`** | Main orchestrator — `run` loop, `step` (scale → dispatch → update servers → metrics), owns the queue and request lifetime. | Touches all of the above; `main` hands it a `LoadBalancer*` and a `SimConfig`. | Single place that wires the OOP pieces into the full discrete-time experiment. |

### Per-class detail

#### `SimConfig`

- **What it does:** Bundles all user-facing sim parameters — time step, run length, initial/min/max servers, scale-up/scale-down queue thresholds, cooldown.
- **Data:** `tickSize`, `duration`, `initialServers`, `minServers`, `maxServers`, `scaleUpThresh`, `scaleDownThresh`, `cooldown`.
- **Methods:** Just a struct with default member init; consumed as a const blob when constructing `Simulator`.
- **Talks to:** `Simulator` (stored as `config_`), and indirectly drives `SimClock`, `AutoScaler`, and the starting `ServerCluster` size.
- **Why it exists:** Matches the Project Scope idea of comparing policies under different settings without editing multiple files.

#### `SimMetrics`

- **What it does:** Running totals for request and time-based stats; computes averages and throughput from those totals.
- **Data:** `totalRequests`, `completedRequests`, `totalWaitTime`, `totalResponseTime`, `totalUptime`.
- **Methods:** `avgWaitTime()`, `avgResponseTime()`, `throughput(duration)`.
- **Talks to:** Gets populated in `Simulator::collectMetrics` from completed `Request` objects and `Server` uptime; `main` reads it after `run()`.
- **Why it exists:** Supports the Iteration #3 "metrics collection" requirement and timeline goals (wait, response, throughput, utilization/cost inputs).

#### `SimClock`

- **What it does:** Steps simulation time forward in discrete increments; exposes current time and tick size.
- **Data:** `tickSize_`, `currentTime_`.
- **Methods:** `tick()`, `advance(dt)`, `reset()`, `getTime()`, `getTickSize()`, `setTickSize()`.
- **Talks to:** `Simulator` drives the loop; time flows into `Server::update`, `AutoScaler::evaluate`, and the request lifecycle.
- **Why it exists:** Every component syncs to one clock (Iteration #3, basic functionality #1).

#### `Request`

- **What it does:** Carries one job's identity, arrival time, service duration, and timestamps for when processing starts and ends.
- **Data:** `id_`, `arrivalTime_`, `serviceTime_`, `startTime_`, `completionTime_`.
- **Methods:** Getters, `setStartTime`, `setCompletionTime`, `isComplete()`.
- **Talks to:** Queued in `RequestQueue`, assigned by `Simulator` to a `Server`, completed requests feed `SimMetrics`.
- **Why it exists:** Models the workload unit so wait and response time can actually be defined and measured.

#### `RequestQueue`

- **What it does:** FIFO queue of pointers to waiting `Request` objects.
- **Data:** `std::queue<Request*>`.
- **Methods:** `enqueue`, `dequeue`, `peek`, `size`, `empty`.
- **Talks to:** Written by arrival logic in `Simulator` (future stochastic generator), read by `AutoScaler` for length, drained by `Simulator` during dispatch.
- **Why it exists:** Represents backlog and the scaling signal described in Project Scope (queue-based autoscaling).

#### `Server`

- **What it does:** Processes one request at a time per tick; decrements remaining service time; tracks uptime and how many requests it's handled.
- **Data:** `id_`, `busy_`, `currentRequest_`, `currentRequestRemainingTime_`, `uptime_`, `requestsProcessed_`.
- **Methods:** `isBusy`, `getId`, `getUptime`, `updateUptime`, `assignRequest`, `update` (returns completed `Request*` or null), `getNumberOfRequestsProcessed`.
- **Talks to:** Gets `Request` from `Simulator`; listed in `ServerCluster`; chosen by `LoadBalancer`.
- **Why it exists:** Localizes "how work gets done" and feeds utilization/uptime into cost metrics (timeline week 3–4).

#### `ServerCluster`

- **What it does:** Owns server objects, assigns IDs, adds/removes machines, ticks all servers for one step, reports idle/busy counts.
- **Data:** `servers_`, `nextId_`.
- **Methods:** `addServer`, `removeServer`, `getServers` (const + non-const), `size`, `idleCount`, `busyCount`, `update`.
- **Talks to:** `Simulator` (dispatch and tick), `LoadBalancer` (selection), `AutoScaler` (scaling), each `Server`.
- **Why it exists:** Wraps the elastic cluster as one object so scaling and balancing stay in sync.

#### `LoadBalancer` (abstract)

- **What it does:** Defines how to pick the next idle server given the current fleet.
- **Data:** None in the base — pure interface.
- **Methods:** `selectServer(std::vector<Server*>&)`, `name()`; virtual destructor.
- **Talks to:** `Server` (via vector), invoked only by `Simulator`.
- **Why it exists:** Strategy pattern — same `Simulator` code, swap in different policies (Iteration #3 load balancing + Project Scope comparison goal).

#### `RoundRobinBalancer` / `LeastConnectionsBalancer`

- **What they do:** Concrete selection rules — round-robin order vs. fewest processed requests.
- **Data:** `RoundRobinBalancer` keeps `lastIndex_`; `LeastConnectionsBalancer` is basically stateless.
- **Methods:** `selectServer`, `name` (both overrides).
- **Talk to:** Same as `LoadBalancer`; no direct coupling beyond `Server*`.
- **Why they exist:** Need at least two strategies to compare experimentally (Project Scope / Week 2 milestone).

#### `AutoScaler`

- **What it does:** Each evaluation, decides whether to add or remove a server based on queue thresholds, min/max fleet size, and how long since the last scale event (cooldown).
- **Data:** `scaleUpQueueThreshold_`, `scaleDownQueueThreshold_`, `cooldownPeriod_`, `minServers_`, `maxServers_`, `lastScaleTime_`.
- **Methods:** `evaluate(cluster, queue, currentTime)`, getters for thresholds/cooldown.
- **Talks to:** `ServerCluster`, `RequestQueue` (read-only size), called from `Simulator::step`.
- **Why it exists:** Keeps autoscaling logic in one place so the main loop doesn't get messy (Iteration #3 item 6; timeline week 3).

#### `Simulator`

- **What it does:** Owns the full simulation lifecycle — builds the cluster from config, runs until duration, and each tick calls the scaler, dispatches the queue through the balancer, advances the cluster, and updates metrics. Also manages `Request` heap ownership via `allRequests_`.
- **Data:** `config_`, `clock_`, `queue_`, `cluster_`, `balancer_`, `scaler_`, `metrics_`, `nextRequestId_`, `allRequests_`.
- **Methods:** `run`, `getMetrics`, `getCurrentTime`; private helpers `step`, `dispatchQueuedRequests`, `collectMetrics`.
- **Talks to:** Everything; `main` supplies the `LoadBalancer` and `SimConfig`.
- **Why it exists:** It's the façade that turns separate classes into the full system from Iteration #3's INPUT / PROCESS / OUTPUT diagram.


## Main functionalities

- **Discrete-time simulation** — Configurable tick size and run duration.
- **Server cluster** — Add/remove servers; each one processes assigned requests over time.
- **Request queue** — FIFO buffer between arrivals and dispatch.
- **Load balancing** — Pluggable strategies via an abstract `LoadBalancer` interface; ships with round-robin and least-connections.
- **Auto-scaling** — Queue-depth thresholds for scale-up/scale-down, min/max fleet size, cooldown to prevent oscillation.
- **Metrics** — Tracks completed requests, aggregate wait/response time, server uptime, derived averages and throughput.
- **CLI** — Pick a balancer and set basic sim parameters from the command line.


## OOP design summary

| Concept | Role in this project |
|--------|----------------------|
| **`Simulator`** | Runs the simulation loop: advances the clock, triggers auto-scaling, dispatches the queue through the balancer, updates servers, and rolls up metrics. |
| **`SimConfig` / `SimMetrics`** | Plain data structures for configuration and aggregated statistics (with a few helper methods on `SimMetrics`). |
| **`LoadBalancer`** | Strategy pattern — `selectServer` and `name` are virtual; `RoundRobinBalancer` and `LeastConnectionsBalancer` provide concrete policies. |
| **`ServerCluster`** | Owns a collection of `Server` instances; supports adding/removing for scaling. |
| **`Server`** | Holds an optional in-progress job, advances processing each tick, exposes utilization/uptime data for metrics. |
| **`Request` / `RequestQueue`** | Model units of work and a FIFO queue between arrival and assignment. |
| **`AutoScaler`** | Encapsulates scale-up/scale-down rules and cooldown, using cluster + queue state. |
| **`SimClock`** | Wraps simulation time and tick duration. |


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

- Build a modular simulation core where you can swap load-balancing strategies and tune auto-scaling parameters without major rewrites.
- Compare policies on wait time, response time, throughput, utilization, and eventually operational cost.
- Support repeatable experiments under different traffic patterns (low, bursty, sustained high) as the implementation matures.
- Keep the codebase and docs clean enough for course or portfolio review.


## GitHub repository purpose

This repo holds the source code, revision history, and documentation. Instructors and reviewers can clone it, build with `make`, track changes over time, and look through commits and this README for design decisions. It also works as a portfolio piece showing systems-style C++ and OOP applied to cloud autoscaling.