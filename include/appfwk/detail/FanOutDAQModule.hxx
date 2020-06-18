
namespace dunedaq::appfwk {

template<typename ValueType>
FanOutDAQModule<ValueType>::FanOutDAQModule(std::string name)
  : DAQModule(name)
  , thread_(std::bind(&FanOutDAQModule<ValueType>::do_work, this))
  , mode_(FanOutMode::NotConfigured)
  , queueTimeout_(100)
  , inputQueue_(nullptr)
  , outputQueues_()
  , wait_interval_us_(std::numeric_limits<size_t>::max())
{

  register_command("configure", &FanOutDAQModule<ValueType>::do_configure);
  register_command("start",  &FanOutDAQModule<ValueType>::do_start);
  register_command("stop",  &FanOutDAQModule<ValueType>::do_stop);
}

template<typename ValueType>
void
FanOutDAQModule<ValueType>::init() {

  auto inputName = configuration_["input"].get<std::string>();
  TLOG(TLVL_DEBUG, "FanOutDAQModule") << get_name() << ": Getting queue with name " << inputName << " as input";
  inputQueue_.reset(new DAQSource<ValueType>(inputName));
  for (auto& output : configuration_["outputs"]) {
    outputQueues_.emplace_back(new DAQSink<ValueType>(output.get<std::string>()));
  }
  
}

template<typename ValueType>
void
FanOutDAQModule<ValueType>::do_configure(const std::vector<std::string>& args)
{
  if (configuration_.contains("fanout_mode")) {
    auto modeString = configuration_["fanout_mode"].get<std::string>();
    if (modeString.find("roadcast") != std::string::npos) {

      mode_ = FanOutMode::Broadcast;
    } else if (modeString.find("irst") != std::string::npos) {

      mode_ = FanOutMode::FirstAvailable;
    } else {
      // RoundRobin by default
      mode_ = FanOutMode::RoundRobin;
    }
  } else {
    // RoundRobin by default
    mode_ = FanOutMode::RoundRobin;
  }

  wait_interval_us_ = configuration_.value<int>("wait_interval", 1000000);

  // auto inputName = configuration_["input"].get<std::string>();
  // TLOG(TLVL_DEBUG, "FanOutDAQModule") << get_name() << ": Getting queue with name " << inputName << " as input";
  // inputQueue_.reset(new DAQSource<ValueType>(inputName));
  // for (auto& output : configuration_["outputs"]) {
  //   outputQueues_.emplace_back(new DAQSink<ValueType>(output.get<std::string>()));
  // }
}

template<typename ValueType>
void
FanOutDAQModule<ValueType>::do_start(const std::vector<std::string>& args)
{
  thread_.start_working_thread_();
}

template<typename ValueType>
void
FanOutDAQModule<ValueType>::do_stop(const std::vector<std::string>& args)
{
  thread_.stop_working_thread_();
}

template<typename ValueType>
void
FanOutDAQModule<ValueType>::do_work()
{
  auto roundRobinNext = outputQueues_.begin();

  // unique_ptr needed since there's no guarantee ValueType has a no-argument
  // constructor
  ValueType data ;

  while (thread_.thread_running()) {
    if (inputQueue_->can_pop()) {

      if (!inputQueue_->pop( data, queueTimeout_)) {
        TLOG(TLVL_WARNING) << get_name() << ": Tried but failed to pop a value from an inputQueue";
        continue;
      }

      if (mode_ == FanOutMode::Broadcast) {
        do_broadcast( data );
      } else if (mode_ == FanOutMode::FirstAvailable) {
        auto sent = false;
        while (!sent) {
          for (auto& o : outputQueues_) {
            if (o->can_push()) {
              auto starttime = std::chrono::steady_clock::now();
              o->push(std::move( data ), queueTimeout_);
              auto endtime = std::chrono::steady_clock::now();

              if (std::chrono::duration_cast<decltype(queueTimeout_)>(endtime - starttime) < queueTimeout_) {
                sent = true;
                break;
              } else {
                TLOG(TLVL_WARNING) << get_name()
                                   << ": A timeout occurred trying to push data "
                                      "onto an outputqueue; data has been lost";
              }
            }
          }
          if (!sent) {
            std::this_thread::sleep_for(std::chrono::microseconds(wait_interval_us_));
          }
        }
      } else if (mode_ == FanOutMode::RoundRobin) {
        while (true) {
          if ((*roundRobinNext)->can_push()) {

            auto starttime = std::chrono::steady_clock::now();
            (*roundRobinNext)->push(std::move( data ), queueTimeout_);
            auto endtime = std::chrono::steady_clock::now();

            if (std::chrono::duration_cast<decltype(queueTimeout_)>(endtime - starttime) >= queueTimeout_) {
              TLOG(TLVL_WARNING) << get_name()
                                 << ": A timeout occurred trying to push data "
                                    "onto an outputqueue; data has been lost";
            }

            ++roundRobinNext;
            if (roundRobinNext == outputQueues_.end())
              roundRobinNext = outputQueues_.begin();
            break;
          } else {
            std::this_thread::sleep_for(std::chrono::microseconds(wait_interval_us_));
          }
        }
      }
    } else { // inputQueue_ is empty
      TLOG(TLVL_TRACE, "FanOutDAQModule") << get_name() << ": Waiting for data";
      std::this_thread::sleep_for(std::chrono::microseconds(wait_interval_us_));
    }
  }
}

} // namespace dunedaq::appfwk

