#ifndef app_framework_base_Services_Logger_hh
#define app_framework_base_Services_Logger_hh

#include <list>
#include <string>

namespace appframework {
	class Logger
	{
	public:
static		void setup(std::list<std::string>) {}
	protected:
		Logger() {}
	};
}

#endif // app_framework_base_Services_Logger_hh