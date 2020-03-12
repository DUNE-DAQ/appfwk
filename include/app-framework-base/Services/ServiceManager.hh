#ifndef app_framework_base_Services_ServiceManager_hh
#define app_framework_base_Services_ServiceManager_hh

namespace appframework {
	class ServiceManager : public Service {


	public:
		static ServiceManager& handle() { static ServiceManager srvm; return srvm; }
		void setup(std::list<std::string>) override {}

	protected:
		ServiceManager() {}
	};
}

#endif // app_framework_base_Services_ServiceManager_hh