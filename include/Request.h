#pragma once

// Representing a request in the system
// The important info is 
// - how long request wait in queue, waitTime
// - how long request takes to complete, responseTime
class Request {
public:
    Request(int id, double arrivalTime, double serviceTime);

    int getId() const;
    double getArrivalTime() const;
    double getServiceTime() const;
    double getWaitTime() const;
    double getResponseTime() const;

    void setStartTime(double startTime);
    void setCompletionTime(double completionTime);

    bool isComplete() const;

private:
    int id_;
    double arrivalTime_;
    double serviceTime_;
    double startTime_;
    double completionTime_;
};
