#include "../include/Request.h"

Request::Request(int id, double arrivalTime, double serviceTime)
    : id_(id),
      arrivalTime_(arrivalTime),
      serviceTime_(serviceTime),
      startTime_(-1.0),
      completionTime_(-1.0) {}

int Request::getId() const {
    return id_;
}

double Request::getArrivalTime() const {
    return arrivalTime_;
}

double Request::getServiceTime() const {
    return serviceTime_;
}

double Request::getWaitTime() const {
    if (startTime_ < 0.0) return -1.0;
    return startTime_ - arrivalTime_;
}

double Request::getResponseTime() const {
    if (completionTime_ < 0.0) return -1.0;
    return completionTime_ - arrivalTime_;
}

void Request::setStartTime(double startTime) {
    startTime_ = startTime;
}

void Request::setCompletionTime(double completionTime) {
    completionTime_ = completionTime;
}

bool Request::isComplete() const {
    return completionTime_ >= 0.0;
}
