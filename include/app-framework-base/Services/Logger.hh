#ifndef app_framework_base_Services_Logger_hh
#define app_framework_base_Services_Logger_hh

#include "app-framework-base/Services/Service.hh"

namespace appframework {
	class Logger : public Service
	{
	public:
		static Logger& handle() { static Logger log; return log; }
		void setup(std::list<std::string>) {}
	protected:
		Logger() {}
	};
}

#endif // app_framework_base_Services_Logger_hh