#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUES_SPSCQUEUE_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUES_SPSCQUEUE_HH_

/**
 *
 * @file An in-process queue using ZeroMQ to pass pointers
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "detail/zmq.hpp"
#include "detail/zmq_context.hh"

#include "app-framework-base/Queues/Queue.hh"

#include <chrono>

namespace appframework {
    
    //==============================================================================
    template <class ValueType, class DurationType = std::chrono::milliseconds>
    class ZeroMQueueSink :   public QueueSink<ValueType, DurationType>
    {
    public:
        using value_type = ValueType;
        using duration_type = DurationType;

        ZeroMQueueSink(const std::string& name)
            : m_socket(appframework::zmq_context(), zmq::socket_type::push)
        {
            m_socket.bind(std::string("inproc://")+name);
        }


        // TODO: Check ZMQ_POLLOUT to see if we can push
        bool can_push() const noexcept override { return m_socket.get(zmq::sockopt::events) & ZMQ_POLLOUT; }

        void push(value_type && val, const duration_type &timeout) override
        {
            value_type* p=new value_type(std::move(val));
            if(!m_socket.send(zmq::buffer(&p, sizeof(p)))){
                delete p;
                throw std::runtime_error("ZeroMQueueSink::push failed");
            }
        }

        ZeroMQueueSink(const ZeroMQueueSink &) = delete;
        ZeroMQueueSink &operator=(const ZeroMQueueSink &) = delete;
        ZeroMQueueSink(ZeroMQueueSink &&) = delete;
        ZeroMQueueSink &operator=(ZeroMQueueSink &&) = delete;

    private:
        zmq::socket_t m_socket;
    };

    //==============================================================================
    template <class ValueType, class DurationType = std::chrono::milliseconds>
    class ZeroMQueueSource :   public QueueSource<ValueType, DurationType>
    {
    public:
        using value_type = ValueType;
        using duration_type = DurationType;

        ZeroMQueueSource(const std::string& name)
            : m_socket(appframework::zmq_context(), zmq::socket_type::pull)
        {
            m_socket.connect(std::string("inproc://")+name);
            m_pollitem=zmq::pollitem_t{m_socket.handle(), 0, ZMQ_POLLIN, 0};
        }

        bool can_pop() const noexcept override { return m_socket.get(zmq::sockopt::events) & ZMQ_POLLIN; }
        value_type pop(const duration_type &timeout) override
        {
            // If there's a message already available on the queue,
            // get it straight away to avoid the slow path of polling
            if(can_pop()){
                return recv_from_socket(m_socket);
            }

            // There wasn't a message available immediately, so we
            // have to wait up to `timeout` via poll()
            zmq::poll(&m_pollitem, 1, timeout);

            if(m_pollitem.revents & ZMQ_POLLIN) {
                return recv_from_socket(m_socket);
            }
            else{
                // We timed out
                throw std::runtime_error("ZeroMQueue::pop timed out");
            }
        }

        ZeroMQueueSource(const ZeroMQueueSource &) = delete;
        ZeroMQueueSource &operator=(const ZeroMQueueSource &) = delete;
        ZeroMQueueSource(ZeroMQueueSource &&) = delete;
        ZeroMQueueSource &operator=(ZeroMQueueSource &&) = delete;

    private:

        value_type recv_from_socket(zmq::socket_t& socket)
        {
            // A message was available: go get it!
            zmq::message_t msg;
            zmq::recv_result_t result=m_socket.recv(msg);
            if(!result){
                throw std::runtime_error("No bytes from socket");
            }
            // The message data is a pointer to the object being passed
            value_type* val_p=*msg.data<value_type*>();
            value_type val(std::move(*val_p));
            delete val_p;
            return val;
        }

        zmq::socket_t m_socket;
        zmq::pollitem_t m_pollitem;
    };
} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUES_SPSCQUEUE_HH_
