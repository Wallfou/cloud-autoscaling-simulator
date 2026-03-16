#include "../include/Server.h"

Server::Server(int id)
    : id_(id),
      busy_(false),
      currentRequest_(nullptr),
      currentRequestRemainingTime_(0.0),
      uptime_(0.0),
      requestsProcessed_(0) {}

void Server::assignRequest(Request* request, double currentTime) {
    busy_ = true;
    currentRequest_ = request;
    currentRequestRemainingTime_ = request->getServiceTime();
    request->setStartTime(currentTime);
}

Request* Server::update(double currentTime, double dt) {
    if (!busy_) return nullptr;

    currentRequestRemainingTime_ -= dt;
    if (currentRequestRemainingTime_ <= 0.0) {
        currentRequest_->setCompletionTime(currentTime);
        Request* completed = currentRequest_;
        currentRequest_ = nullptr;
        busy_ = false;
        requestsProcessed_++;
        return completed;
    }
    return nullptr;
}

bool Server::isBusy() const {
    return busy_;
}

int Server::getId() const {
    return id_;
}

double Server::getUptime() const {
    return uptime_;
}

void Server::updateUptime(double dt) {
    if (busy_) uptime_ += dt;
}

int Server::getNumberOfRequestsProcessed() const {
    return requestsProcessed_;
}
