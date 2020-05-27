/*
 * DAQSource.hh
 *
 *  Created on: 20 May 2020
 *      Author: glehmann
 */

#ifndef DAQSINK_HH_
#define DAQSINK_HH_


#include <string>
#include <memory>
#include <chrono>
#include <typeinfo>
#include <app-framework/QueueRegistry.hh>
#include <app-framework-base/NamedQueueI.hh>

namespace appframework {

template <typename T>
class DAQSource {
public:
    using value_type = T;
    using duration_type = std::chrono::milliseconds;

	DAQSource(std::string name);
	T pop(const duration_type& timeout=duration_type::zero());
	bool can_pop();

private:
	std::shared_ptr<NamedQueueI<T>> queue_;
};

template <typename T>
DAQSource<T>::DAQSource(std::string name) {
	queue_ = QueueRegistry::get()->get_queue<T>(name);
    if (!queue_)
        throw std::bad_cast();
}

template <typename T>
T DAQSource<T>::pop(const duration_type& timeout) {
	return queue_->pop(timeout);
}

template <typename T>
bool DAQSource<T>::can_pop() {
	return queue_->can_pop();
}

} // namespace appframework

#endif /* DAQSINK_HH_ */
