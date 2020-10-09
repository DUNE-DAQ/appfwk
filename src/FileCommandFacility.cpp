/** @file FileCommands.cpp

    Fixme: the local "streams" types supporting FileCommandSource are
    general and could be useful broken out.

    Fixme: need more imp files with http://, zeromq://, gRPC://, etc.

 */

#include "appfwk/CommandFacility.hpp"
#include "appfwk/Issues.hpp"
#include "appfwk/DAQModuleManager.hpp"

#include <cetlib/BasicPluginFactory.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <string>
#include <memory>

using namespace dunedaq::appfwk;

using object_t = nlohmann::json;

// Abstract the object level
struct ObjectStream {
    std::string name;
    std::iostream& io;

    ObjectStream(std::string name, std::iostream& io) : name(name), io(io) {
        if (!io) {
            throw BadFile(ERS_HERE, name,"");
        }
    }
    virtual ~ObjectStream(){}

    // One could, but in this case, one should not.
    ObjectStream(const ObjectStream&) = delete;
    ObjectStream(const ObjectStream&&) = delete;
    ObjectStream& operator=(const ObjectStream&) = delete;
    ObjectStream& operator=(const ObjectStream&&) = delete;

    // Get the next object from the stream.
    virtual object_t get() = 0;

    // Put an object to the stream.
    virtual void put(object_t) = 0;

    // For object streams that buffer, may expclictly flush prior to
    // destruction.
    virtual void flush() {}

    // Get stream, checked okay for reading
    std::iostream& r() {
        if (io.eof()) {
            ERS_INFO("EOF: " << name);
            throw StreamExhausted(ERS_HERE, name, "EOF");
        }
        if (! io.good()) {
            throw InternalError(ERS_HERE, "stream bad: " + name);
        }
        return io;
    }

    // Get stream, checked okay for writing
    std::iostream& w() {
        if (! io.good()) {
            throw InternalError(ERS_HERE, "stream bad: " + name);
        }
        return io;
    }

};


// Interpret byte stream as a JSON stream.
// https://en.wikipedia.org/wiki/JSON_streaming.
struct JsonStream : ObjectStream {   
    virtual ~JsonStream() {}
    JsonStream(std::string name, std::iostream& io) : ObjectStream(name, io) {}
    // One could, but in this case, one should not.
    JsonStream(const JsonStream&) = delete;
    JsonStream(const JsonStream&&) = delete;
    JsonStream& operator=(const JsonStream&) = delete;
    JsonStream& operator=(const JsonStream&&) = delete;


    virtual object_t get() {
        object_t obj;
        try {
            r() >> obj;
        }
        catch (const object_t::parse_error& pe) {
            if (pe.id == 101) {
                ERS_INFO("EOF: " << name);
                throw StreamExhausted(ERS_HERE, name, "EOF");
            }
            throw StreamCorrupt(ERS_HERE, name, pe.what());
        }
        if (! obj.is_object()) {
            std::string msg = "want: object, got: ";
            msg += obj.dump(4); // fixme: temp for debugging!
            throw StreamCorrupt(ERS_HERE, name, msg);
        }
        return obj;
    }
    
    virtual void put(object_t obj) {
        w() << obj;             // what could go wrong?
    }
};

// Interpret a byte stream as a JSON Array.  This slurps input or
// buffers output.  Multiple calls to flush() will effectively produce
// a JSON Stream of arrays of objects.
struct JsonArray : public ObjectStream {
    object_t arr;
    bool isread;
    JsonArray(std::string name, std::iostream& io, bool isread=true)
        : ObjectStream(name, io)
        , isread(isread) {
        arr = object_t::array();
        if (isread) slurp();
    }
    virtual ~JsonArray() { if (!isread) { this->flush(); } }

    // One could, but in this case, one should not.
    JsonArray(const JsonArray&) = delete;
    JsonArray(const JsonArray&&) = delete;
    JsonArray& operator=(const JsonArray&) = delete;
    JsonArray& operator=(const JsonArray&&) = delete;

    void slurp() {
        try {
            r() >> arr;
        }
        catch (const object_t::parse_error& pe) {
            throw StreamCorrupt(ERS_HERE, name, pe.what());
        }
        if (! arr.is_array()) {
            std::string msg = "want: array, got: ";
            msg += arr.dump(4); // fixme: temp for debugging!
            throw StreamCorrupt(ERS_HERE, name, msg);
        }
    }

    virtual object_t get() {
        if (arr.empty()) {
            ERS_INFO("EOF: " << name);
            throw StreamExhausted(ERS_HERE, name, "array end");
        }
        auto obj = arr[0];
        arr.erase(0);
        if (! obj.is_object()) {
            std::string msg = "want: object, got: ";
            msg += obj.dump(4); // fixme: temp for debugging!
            throw StreamCorrupt(ERS_HERE, name, msg);
        }
        return obj;
    }

    virtual void put(object_t obj) {
        arr.push_back(obj);
    }

    virtual void flush() {
        if (arr.empty()) { return; }
        w() << arr;
        arr.empty();
    }
};


// Note: this is registered as fileCommandFacility (lower-case "f"),
// available on CLI as file:// scheme or simple filesystem path name.
// Since files are not bi-directional, this will form an output
// filename based on the input with "-out" appended to the base
// filename.  The same extention and thus format assumptions used by
// the input are kept for output.
struct fileCommandFacility : public CommandFacility {

    std::fstream istr;
    std::unique_ptr<ObjectStream> ios;

    virtual ~fileCommandFacility() {
        // assure these die first.
        ios.reset();
    }
    fileCommandFacility(const fileCommandFacility&) = delete;
    fileCommandFacility(const fileCommandFacility&&) = delete;
    fileCommandFacility& operator=(const fileCommandFacility&) = delete;
    fileCommandFacility& operator=(const fileCommandFacility&&) = delete;

    fileCommandFacility(std::string uri) : CommandFacility(uri) {

        auto dot = uri.find_last_of(".");
        auto sep = uri.find("://");
        if (dot == std::string::npos) {
            throw UnsupportedUri(ERS_HERE, uri);
        }

        std::string ext = uri.substr(dot+1);
        std::string scheme = "";
        std::string iname = uri;

        if (sep == std::string::npos) { // simple path
            scheme = "file";
            iname = uri;
        } else {                  // with scheme
            scheme = uri.substr(0, sep);
            iname = uri.substr(sep+3);
        }
        std::string bname = iname.substr(0, dot);

        ERS_INFO("open: scheme:" << scheme << " ext:" << ext);

        
        if (scheme.empty() || scheme == "file") {
            istr.open(iname, std::ios_base::in);
            if (ext == "json") {
                ios.reset(new JsonArray(iname, istr, true));
            }
            if (ext == "jstream") {
                ios.reset(new JsonStream(iname, istr));
            }
            return;
        }
        throw UnsupportedUri(ERS_HERE, uri);
    }


    object_t recv() const {
        return ios->get();
    }


    void run(DAQModuleManager& manager) const {

        while (true) {
            object_t command;

            try {
                command = this->recv();
            }
            catch (const StreamExhausted& e) {
                ERS_INFO("Command stream end");
                break;
            }

            manager.execute(command);
            ERS_INFO("DAQModuleManager execution complete");
        }

    }

};

extern "C" {
    std::shared_ptr<dunedaq::appfwk::CommandFacility> make(std::string uri) { 
        return std::shared_ptr<dunedaq::appfwk::CommandFacility>(new fileCommandFacility(uri));
    }
}
