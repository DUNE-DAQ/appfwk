#include "app-framework/Queues/StdDeQueue.hh"

namespace appframework {
typedef StdDeQueue<std::vector<int>> VectorIntStdDeQueue;
}
DEFINE_DUNE_BUFFER(appframework::VectorIntStdDeQueue)