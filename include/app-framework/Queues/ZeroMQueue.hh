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


        bool can_push() const noexcept override { return true; }
        void push(value_type && val, const duration_type &timeout) override
        {
            m_sender.send_to_socket(m_socket, std::move(val));
        }

        ZeroMQueueSink(const ZeroMQueueSink &) = delete;
        ZeroMQueueSink &operator=(const ZeroMQueueSink &) = delete;
        ZeroMQueueSink(ZeroMQueueSink &&) = delete;
        ZeroMQueueSink &operator=(ZeroMQueueSink &&) = delete;

    private:
        //==============================================================================

        // A class to hold the implementation of recv_from_socket for
        // general types. Specialized for std::unique_ptr<T>
        // below. There's probably a nicer way to do this (with
        // std::enable_if perhaps?), but I can't work out how to spell it
        template <class T>
        class SocketSender
        {
        public:
            void send_to_socket(zmq::socket_t& socket, T&& val)
            {
                value_type* p=new value_type(std::move(val));
                if(!socket.send(zmq::buffer(&p, sizeof(p)))){
                    delete p;
                    throw std::runtime_error("ZeroMQueueSink::push failed");
                }

            }
        };

        // Specialization of recv_from_socket for std::unique_ptr<T>
        // which saves a little bit of work
        template<class T>
        class SocketSender<std::unique_ptr<T>>
        {
        public:
            using ptr_type = std::unique_ptr<T>;
            using pointee_type = T;

            void send_to_socket(zmq::socket_t& socket, ptr_type&& val)
            {
                auto raw_pointer=val.release();
                if(!socket.send(zmq::buffer(&raw_pointer, sizeof(raw_pointer)))){
                    throw std::runtime_error("ZeroMQueueSink::push failed");
                }
            }
        };

        SocketSender<value_type> m_sender;
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
                return m_receiver.recv_from_socket(m_socket);
            }

            // There wasn't a message available immediately, so we
            // have to wait up to `timeout` via poll()
            zmq::poll(&m_pollitem, 1, timeout);

            if(m_pollitem.revents & ZMQ_POLLIN) {
                // A message was available: go get it!
                return m_receiver.recv_from_socket(m_socket);
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

        //==============================================================================

        // A class to hold the implementation of recv_from_socket for
        // general types. Specialized for std::unique_ptr<T>
        // below. There's probably a nicer way to do this (with
        // std::enable_if perhaps?), but I can't work out how to spell it
        template <class T>
        class SocketReceiver
        {
        public:

            T recv_from_socket(zmq::socket_t& socket)
            {
                zmq::message_t msg;
                zmq::recv_result_t result=socket.recv(msg);
                if(!result){
                    throw std::runtime_error("No bytes from socket");
                }
                // The message data is a pointer to the object being passed
                T* val_p=*msg.data<T*>();
                T val(std::move(*val_p));
                delete val_p;
                return val;
            }
        };

        // Specialization of recv_from_socket for std::unique_ptr<T>
        // which saves a little bit of work
        template<class T>
        class SocketReceiver<std::unique_ptr<T>>
        {
        public:
            using ptr_type = std::unique_ptr<T>;
            using pointee_type = T;

            ptr_type recv_from_socket(zmq::socket_t& socket)
            {
                zmq::message_t msg;
                zmq::recv_result_t result=socket.recv(msg);
                if(!result){
                    throw std::runtime_error("No bytes from socket");
                }
                return ptr_type(*msg.data<T*>());
            }
        };

        SocketReceiver<value_type> m_receiver;

        zmq::socket_t m_socket;
        zmq::pollitem_t m_pollitem;
    };
} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUES_SPSCQUEUE_HH_
