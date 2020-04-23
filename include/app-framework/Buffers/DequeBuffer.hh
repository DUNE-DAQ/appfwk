#ifndef app_framework_Buffers_DequeBuffer_hh
#define app_framework_Buffers_DequeBuffer_hh

#include "app-framework-base/Buffers/Buffer.hh"

#include <unistd.h>
#include <atomic>
#include <deque>
#include <functional>
#include <mutex>

namespace appframework {

template <class T>
class DequeBuffer : virtual public BufferInput<T>, virtual public BufferOutput<T> {
   public:
    DequeBuffer();

    void Configure();

    size_t size() { return fSize.load(); }
    size_t capacity() { return fCapacity; }

    bool empty() { return size() == 0; }
    bool full() { return size() >= capacity(); }

    int push(T&&);  /// push one on, return new size if successful, -1 if not
    T pop();        /// pop one off, return object

   private:
    std::deque<T> fDeque;
    std::atomic<size_t> fSize = 0;
    size_t fCapacity;

    std::mutex fMutex;
    size_t fRetryTime_ms;
    size_t fPushRetries;
    size_t fPopRetries;
};

}  // namespace appframework

// Template function definitions


template <class T>
appframework::DequeBuffer<T>::DequeBuffer() {
    Configure();
}

template <class T>
void appframework::DequeBuffer<T>::Configure() {
    BufferI::fAttributes.isBounded = true;
    BufferI::fAttributes.isSearchable = false;

    fDeque = std::deque<T>();
    fSize = 0;

    if (BufferI::attributes().isBounded) {
        fCapacity = 1000;
    } else {
        fCapacity = fDeque.max_size();
    }

    BufferInput<T>::fPushTimeout_ms = 1e6;
    BufferOutput<T>::fPopTimeout_ms = 1e7;
    fRetryTime_ms = 100;

    fPushRetries = BufferInput<T>::fPushTimeout_ms / fRetryTime_ms;
    fPopRetries = BufferOutput<T>::fPopTimeout_ms / fRetryTime_ms;
}


template <class T>
int appframework::DequeBuffer<T>::push(T&& obj) {
    int n_retries = 0;
    while (full()) {
        if (n_retries == fPushRetries) {
            // could throw, but here choose to return -1
            // throw std::runtime_error("Reached Max Push Retries in DequeBuffer");
            return -1;
        }
        ++n_retries;
        usleep(fRetryTime_ms);
    }

    if (full()) throw std::runtime_error("Full after check? Another thread accessing?");
    std::lock_guard<std::mutex> lck(fMutex);
    fDeque.push_back(std::move(obj));
    fSize++;
    return size();
}

template <class T>
T appframework::DequeBuffer<T>::pop() {
    int n_retries = 0;
    while (empty()) {
        if (n_retries == fPopRetries) {
            throw std::runtime_error("No Data: Reached Max Pop Retries in DequeBuffer");
            // return -1;
        }
        ++n_retries;
        usleep(fRetryTime_ms);
    }

    if (empty()) throw std::runtime_error("Empty after check? Another thread accessing?");

    std::lock_guard<std::mutex> lck(fMutex);
    T obj(std::move(fDeque.front()));
    fDeque.pop_front();
    fSize--;
    return obj;
}

#endif  // app_framework_Buffers_Buffer_hh
