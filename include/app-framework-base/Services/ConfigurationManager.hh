#ifndef app_framework_base_Services_ConfigurationManager_hh
#define app_framework_base_Services_ConfigurationManager_hh

namespace appframework {
	
	class ConfigurationManager : public Service {

	public:
		static ConfigurationManager& handle() { static ConfigurationManager conf; return conf; }
		void setup(std::list<std::string>) override {}

	protected:
		ConfigurationManager() {}
	};
}

#endif // app_framework_base_Services_ConfigurationManager_hh