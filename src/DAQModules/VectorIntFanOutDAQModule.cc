#include "app-framework/DAQModules/FanOutDAQModule.hh"

namespace appframework {
typedef appframework::FanOutDAQModule<std::vector<int>>
  VectorIntFanOutDAQModule;
}

DEFINE_DUNE_DAQ_MODULE(appframework::VectorIntFanOutDAQModule)