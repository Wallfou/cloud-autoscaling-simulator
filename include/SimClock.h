#pragma once

class SimClock {
public:
    explicit SimClock(double tickSize = 1.0);

    void tick();
    void advance(double dt);
    void reset();

    // getters n setters
    double getTime() const;
    double getTickSize() const;
    void setTickSize(double tickSize);

private:
    double tickSize_;
    double currentTime_;
};
