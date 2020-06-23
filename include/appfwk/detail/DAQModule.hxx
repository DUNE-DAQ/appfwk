namespace dunedaq::appfwk {

template <typename Child>
void
DAQModule::register_command(const std::string &name, void (Child::*f)(const std::vector<std::string>&)) {
  using namespace std::placeholders;

  bool done = commands_.emplace(name, std::bind(f, dynamic_cast<Child*>(this), _1)).second;
  if (!done) {
    // Throw here
    throw CommandRegistrationFailed(ERS_HERE, get_name(), name);
  }
}

} // namespace dunedaq::appfwk

