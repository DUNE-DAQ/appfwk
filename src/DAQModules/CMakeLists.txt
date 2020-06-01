
#file(GLOB source_files *.cc)
#add_library(app-framework_DAQModules SHARED ${source_files} )

add_library(app-framework_DebugLoggingDAQModule_duneObject DebugLoggingDAQModule.cc)
add_library(app-framework_FakeDataConsumerDAQModule_duneObject FakeDataConsumerDAQModule.cc)
add_library(app-framework_FakeDataProducerDAQModule_duneObject FakeDataProducerDAQModule.cc)
