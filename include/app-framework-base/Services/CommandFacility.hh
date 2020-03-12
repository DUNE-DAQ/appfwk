#ifndef app_framework_core_Services_CommandFacility_hh
#define app_framework_core_Services_CommandFacility_hh

namespace appframework {
	class CommandFacility : public Service {
	public:
		static CommandFacility& handle() { static CommandFacility comm; return comm; }
		void setup(std::list<std::string>) override {}
		std::future<int> listen();

	protected:
		CommandFacility() {}
	};
}

#endif // app_framework_core_Services_CommandFacility_hh