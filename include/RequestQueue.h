#pragma once

#include <queue>
#include "Request.h"

// A queue of requests
// Requests are added to queue on arrival and then they wait untill assigned to a server
class RequestQueue {
public:
    void enqueue(Request* request);
    Request* dequeue();
    Request* peek() const;

    int size() const;
    bool empty() const;

private:
    std::queue<Request*> queue_;
};
