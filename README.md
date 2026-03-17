# Cloud Autoscaling Simulator

## Project Scope

This project will design and implement a cloud infrastructure simulator that models how distributed server clusters handle incoming workloads.

The proposed project will ideally:
- Generate incoming client requests using stochastic arrival models
- Simulate multiple servers handling queued requests
- Implement load balancing strategies
- Automatically scale servers up or down based on system load
- Measure cost vs performance tradeoffs

The objective is to analyze how different auto-scaling and load balancing policies impact latency, throughput, resource utilization, and operational cost.

## Project Objectives

(Individual, since I am working alone)

- Design a modular cloud workload simulation framework.
- Implement request arrival simulation using Poisson or random distributions.
- Implement at least two load balancing strategies.
- Implement dynamic auto-scaling logic (playing around with queue length and utilization)
- Compare results in terms of response time, request wait time, server utilization, and operational cost (which could be based on server uptime).
- Produce analysis for different workloads

## Technologies and Tools

**Programming Language:**
- C++

**Development Tools**
- GitHub for version control
- g++ compiler
- Makefile
- VS Code

## Project Timeline

### Sprint 1

Todos:
- Finalize system architecture
- Define class structures
- Implement a discrete-time simulation clock with different tick size
- Setup Github repo

Milestone: need a functional skeleton system

---

### Sprint 2

Todos:
- Implement at least two load balancing strategies (need more research to see decide)
- Integrate strategies into the simulation engine
- Add request arrival generation (this could be random/Poisson-like)

Milestone: implementing multi-strategy load balancing system

Deliverable: CLI flag to choose balancer, sample run output

---

### Sprint 3

Todos:
- Implement a threshold for scaling up (probably based on queue length or server utilization)
- Implement a threshold for scaling down
- Add a cool-down timer to prevent too much oscillation
- Track server uptime for operational cost calculation

Milestone: completing cloud simulation

Deliverable: autoscaling working

---

### Sprint 4

Todos:
- Compute necessary data for analysis: response time, waiting time, throughput, server utilization, operational cost
- Run experiments on low traffic, traffic in bursts, and sustained high traffic scenarios.
- Compare load balancing strategies and their results to determine the best strategy
- Export results to csv for plotting

Milestone: Experimental evaluation complete

Deliverable: csv export and comparison plot or tables

---

### Sprint 5

Todos:
- Clean code
- Add documentation
- Write the final PDF document to include any necessary information about the project's outcome and process
- Write the final README file with build instructions and example outputs

Milestone: Polished final product

Deliverable: pdf, readme and clean builds
