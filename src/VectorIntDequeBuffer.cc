#include "app-framework/Buffers/DequeBuffer.hh"

namespace appframework {
typedef DequeBuffer<std::vector<int>> VectorIntDequeBuffer;
}
DEFINE_DUNE_BUFFER(appframework::VectorIntDequeBuffer)