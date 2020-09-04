
namespace dunedaq::appfwk {

template<typename ValueType>
FanOutDAQModule<ValueType>::FanOutDAQModule(std::string name)
  : DAQModule(name)
  , thread_(std::bind(&FanOutDAQModule<ValueType>::do_work, this, std::placeholders::_1))
  , mode_(FanOutMode::NotConfigured)
  , queueTimeout_(100)
  , inputQueue_(nullptr)
  , outputQueues_()
  , wait_interval_us_(std::numeric_limits<size_t>::max())
{

  register_command("configure", &FanOutDAQModule<ValueType>::do_configure);
  register_command("start", &FanOutDAQModule<ValueType>::do_start);
  register_command("stop", &FanOutDAQModule<ValueType>::do_stop);
}

template<typename ValueType>
void
FanOutDAQModule<ValueType>::init(const nlohmann::json& init_data)
{

  auto inputName = init_data["input"].get<std::string>();
  TLOG(TLVL_TRACE, "FanOutDAQModule") << get_name() << ": Getting queue with name " << inputName << " as input";
  inputQueue_.reset(new DAQSource<ValueType>(inputName));
  for (auto& output : init_data["outputs"]) {
    outputQueues_.emplace_back(new DAQSink<ValueType>(output.get<std::string>()));
  }
}

template<typename ValueType>
void
FanOutDAQModule<ValueType>::do_configure(const std::vector<std::string>& /*args*/)
{
  // if (get_config().contains("fanout_mode")) {
  //   auto modeString = get_config()["fanout_mode"].get<std::string>();
  //   if (modeString == "broadcast") {

  //     mode_ = FanOutMode::Broadcast;
  //   } else if (modeString == "first_available") {

  //     mode_ = FanOutMode::FirstAvailable;
  //   } else if (modeString == "round_robin") {
  //     mode_ = FanOutMode::RoundRobin;
  //   } else {
  //     throw ConfigureFailed(ERS_HERE, get_name(), std::string("given unknown fanout_mode ") + modeString);
  //   }
  // } else {
  //   // RoundRobin by default
  //   mode_ = FanOutMode::RoundRobin;
  // }

  // wait_interval_us_ = get_config().value<int>("wait_interval_us", 10000);
  // queueTimeout_ = std::chrono::milliseconds(get_config().value<int>("queue_timeout_ms", 100));
}

template<typename ValueType>
void
FanOutDAQModule<ValueType>::do_start(const std::vector<std::string>& /*args*/)
{
  thread_.start_working_thread();
}

template<typename ValueType>
void
FanOutDAQModule<ValueType>::do_stop(const std::vector<std::string>& /*args*/)
{
  thread_.stop_working_thread();
}

template<typename ValueType>
void
FanOutDAQModule<ValueType>::do_work(std::atomic<bool>& running_flag)
{
  auto roundRobinNext = outputQueues_.begin();

  ValueType data;

  while (running_flag.load()) {
    if (inputQueue_->can_pop()) {

      try {
        inputQueue_->pop(data, queueTimeout_);
      } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
        continue;
      }

      if (mode_ == FanOutMode::Broadcast) {
        do_broadcast(data);
      } else if (mode_ == FanOutMode::FirstAvailable) {
        auto sent = false;
        while (!sent) {
          for (auto& o : outputQueues_) {
            if (o->can_push()) {
	      try {
		o->push(std::move(data), queueTimeout_);
		sent = true;
	      } catch(const dunedaq::appfwk::QueueTimeoutExpired& ) {
		ers::warning(BroadcastFailed(ERS_HERE,
					     get_name(),
					     "Timeout occurred trying to broadcast data to "
					     "output queue; data may be lost if it doesn't "
					     "make it into any other output queues, either"));
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

            (*roundRobinNext)->push(std::move(data), queueTimeout_);

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
