/**
 * @file DummyModule.cxx DummyModule class implementation. Contains only macro call needed to create C entry point for
 * dynamic loading.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DummyModule.hpp"

DEFINE_DUNE_DAQ_MODULE(dunedaq::appfwk::DummyModule)
