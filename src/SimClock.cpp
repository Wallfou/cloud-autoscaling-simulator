#include "../include/SimClock.h"

SimClock::SimClock(double tickSize) : tickSize_(tickSize), currentTime_(0.0) {}

void SimClock::tick() {
    currentTime_ += tickSize_;
}

void SimClock::advance(double dt) {
    currentTime_ += dt;
}

void SimClock::reset() {
    currentTime_ = 0.0;
}

double SimClock::getTime() const {
    return currentTime_;
}

double SimClock::getTickSize() const {
    return tickSize_;
}

void SimClock::setTickSize(double tickSize) {
    tickSize_ = tickSize;
}
