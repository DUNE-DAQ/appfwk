#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUES_DETAIL_ZMQ_CONTEXT_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUES_DETAIL_ZMQ_CONTEXT_HH_

#include "zmq.hpp"

namespace appframework
{
    // A little function to get a singleton ZeroMQ context
    zmq::context_t& zmq_context()
    {
        // Apparently the C++ standard guarantees that static
        // variables are constructed in a thread-safe way, which is
        // nice
        static zmq::context_t ctx;
        return ctx;
    }
}

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUES_DETAIL_ZMQ_CONTEXT_HH_
