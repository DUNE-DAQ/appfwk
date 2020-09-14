/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains struct and other type definitions for shema in 
 * namespace dunedaq::appfwk::fdc.
 */
#ifndef DUNEDAQ_APPFWK_FDC_STRUCTS_HPP
#define DUNEDAQ_APPFWK_FDC_STRUCTS_HPP


#include <string>

namespace dunedaq::appfwk::fdc {

    // @brief A count of very many things
    using Size = uint64_t;

    // @brief A count of not too many things
    using Count = int32_t;

    // @brief Fake Data Consumer DAQ Module Configuration
    struct Conf {

        // @brief Number of numbers
        Size nIntsPerVector;

        // @brief Number to start with
        Count starting_int;

        // @brief Number to end with
        Count ending_int;

        // @brief Milliseconds to wait on queue before timing out
        Count queue_timeout_ms;
    };

    // @brief An identifier
    using Ident = std::string;

    // @brief 
    struct Init {

        // @brief 
        Ident input;
    };

} // namespace dunedaq::appfwk::fdc

#endif // DUNEDAQ_APPFWK_FDC_STRUCTS_HPP