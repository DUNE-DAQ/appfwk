#include "app-framework/UserModules/FanOutUserModule.hh"

namespace appframework {
typedef appframework::FanOutUserModule<std::vector<int>>
  VectorIntFanOutUserModule;
}

DEFINE_DUNE_USER_MODULE(appframework::VectorIntFanOutUserModule)