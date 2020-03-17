#ifndef app_framework_base_Services_ServiceManager_hh
#define app_framework_base_Services_ServiceManager_hh

#include <app-framework-base/Services/Service.hh>

namespace appframework {
	class ServiceManager {
	public:
		static void setup(std::list<std::string>) {}
		static ServiceManager* handle() { if (!handle_) handle_ = new ServiceManager(); return handle_; }

		std::unique_ptr<Service> const& getService(std::string service_name) { return service_map_[service_name]; }
		void register_service(std::string name, std::unique_ptr<Service>&& service) { service_map_[name] = std::move(service); }

	private:
		ServiceManager() {}

	private:
		std::map<std::string, std::unique_ptr<Service>> service_map_;
		static ServiceManager* handle_;
	};
}

#endif // app_framework_base_Services_ServiceManager_hh