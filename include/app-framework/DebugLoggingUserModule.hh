#ifndef APP_FRAMEWORK_DEBUGLOGGINGUSERMODULE_HH
#define APP_FRAMEWORK_DEBUGLOGGINGUSERMODULE_HH

namespace appframework {
	class DebugLoggingUserModule : public UserModule {
	public:
		void execute_transition(TransitionName cmd) override;
	private:
		std::string transitionNameToString(TransitionName cmd);
	};
}

#endif // APP_FRAMEWORK_DEBUGLOGGINGUSERMODULE_HH