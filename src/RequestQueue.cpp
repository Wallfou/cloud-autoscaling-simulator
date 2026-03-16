#include "../include/RequestQueue.h"

void RequestQueue::enqueue(Request* request) {
    queue_.push(request);
}

Request* RequestQueue::dequeue() {
    if (queue_.empty()) return nullptr;
    Request* poppedRequest = queue_.front();
    queue_.pop();
    return poppedRequest;
}

Request* RequestQueue::peek() const {
    if (queue_.empty()) return nullptr;
    return queue_.front();
}

int RequestQueue::size() const {
    return queue_.size();
}

bool RequestQueue::empty() const {
    return queue_.empty();
}
