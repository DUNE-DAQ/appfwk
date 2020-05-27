/*
 * DAQSink.hh
 *
 *  Created on: 19 May 2020
 *      Author: glehmann
 */

#ifndef DAQSOURCE_HH_
#define DAQSOURCE_HH_


#include <string>
#include <memory>
#include <chrono>
#include <app-framework/QueueRegistry.hh>
#include <app-framework-base/NamedQueueI.hh>

namespace appframework {

template <typename T>
class DAQSink {
public:
    using value_type = T;
    using duration_type = std::chrono::milliseconds;

	DAQSink(std::string name);
	void push(T element, const duration_type& timeout=duration_type::zero());
	bool can_push();

private:
	std::shared_ptr<NamedQueueI<T>> queue_;
};

template <typename T>
DAQSink<T>::DAQSink(std::string name) {
	queue_ = QueueRegistry::get()->get_queue<T>(name);
    if (!queue_)
        throw std::bad_cast();
}

template <typename T>
void DAQSink<T>::push(T element, const duration_type& timeout) {
	queue_->push(element, timeout);
}

template <typename T>
bool DAQSink<T>::can_push() {
	return queue_->can_push();
}

} // namespace appframework

#endif /* DAQSOURCE_HH_ */
