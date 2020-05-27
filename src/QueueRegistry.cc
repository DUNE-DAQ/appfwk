#include "app-framework/QueueRegistry.hh"

namespace appframework {

QueueRegistry* QueueRegistry::me_ = nullptr;

QueueRegistry::QueueRegistry() {
}


QueueRegistry::~QueueRegistry() {
}

QueueRegistry* QueueRegistry::get() {
    if ( !me_ ) {
        me_ = new QueueRegistry();
    }
    return me_;
}

}