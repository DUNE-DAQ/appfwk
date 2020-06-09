/**
 * @file Logger.hpp Logger interface definition
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_LOGGER_HPP_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_LOGGER_HPP_

#include <string>
#include <vector>
#include <ers/ers.h>

#define TRACE_LOG_FUN_PROTO \
  static void erstrace_user(struct timeval *, int, uint8_t, const char*, const char*, int, const char*, uint16_t nargs, const char *msg, ...); \
  static void erstrace_user(struct timeval *, int, uint8_t, const char*, const char*, int, const char*, uint16_t nargs, const std::string& msg, ...)
#undef TRACE_LOG_FUNCTION
#define TRACE_LOG_FUNCTION erstrace_user
#define TRACE_4_LVLSTRS "ftl","err","wrn","nfo"
#	define TRACE_60_LVLSTRS  "log", "dbg", "d01", "d02", "d03", "d04", "d05", "d06", "d07", "d08", "d09", \
							 "d10", "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19", \
							 "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29", \
							 "d30", "d31", "d32", "d33", "d34", "d35", "d36", "d37", "d38", "d39", \
							 "d40", "d41", "d42", "d43", "d44", "d45", "d46", "d47", "d48", "d49", \
							 "d50", "d51", "d52", "d53", "d54", "d55", "d56", "d57", "d58"
#include <TRACE/trace.h>


namespace dunedaq::appfwk {
/**
 * @brief The Logger class defines the interface necessary to configure central
 * logging within a DAQ Application.
 */
class Logger
{
public:
  /**
   * @brief Setup the Logger service
   * @param args Command-line arguments used to setup the Logger
   */
  static void setup(const std::vector<std::string> & args) {}
};

} // namespace dunedaq::appfwk



namespace {  // unnamed namespace (i.e. static (for each compliation unit only))

	struct ErsFatalStreamer
	{
		inline ErsFatalStreamer &operator<<(const ers::Issue &r)
		{
			ers::fatal( r );
			return *this;
		}
	};  // struct ErsFatalStreamer

	struct ErsErrorStreamer
	{
		inline ErsErrorStreamer &operator<<(const ers::Issue &r)
		{
			ers::error( r );
			return *this;
		}
	};  // struct ErsErrorStreamer

	struct ErsWarningStreamer
	{
		//ErsWarningStreamer( ers::LocalContext c ) : _ctx(c) {}
		//ers::LocalContext _ctx;
		inline ErsWarningStreamer &operator<<(const ers::Issue &r)
		{
			// would be nice of the "context" could be created from the streamer instance
			ers::warning( r );
			return *this;
		}
	};  // struct ErsWarningStreamer

}  // unnamed namespace




#undef TLOG_ERROR
#undef TLOG_WARNING

// The ERS_HERE macro could/should be in the constructor.
#define ERS_FATAL()   ErsFatalStreamer()
#define ERS_ERROR()   ErsErrorStreamer()
#define ERS_WARNING() ErsWarningStreamer()



#undef TLVL_ERROR
//#undef TLVL_WARNING
#undef TLVL_INFO
#undef TLVL_DEBUG
//#undef TLVL_TRACE
// out with the defaults (above), in with the specifics (below)
#define TLVL_INFO  0   // to be shifted up by 3 later
#define TLVL_LOG   1   // to be shifted up by 3 later
#define TLVL_DEBUG 2   // to be shifted up by 3 later

#define SL_FRC(lvl) ((lvl)==0 || (lvl)==1)

#if defined(__has_feature)
#  if __has_feature(thread_sanitizer)
__attribute__((no_sanitize("thread")))
#  endif
#endif
static void verstrace_user(struct timeval *, int TID, uint8_t lvl, const char* insert
	, const char* file, int line, const char* function, uint16_t nargs, const char *msg, va_list ap)
{
	size_t printed = 0;
	int    retval;
	const char *outp;
		char   obuf[TRACE_USER_MSGMAX];

	if ((insert && (printed = strlen(insert))) || nargs)
	{
		/* check insert 1st to make sure printed is set */
		// assume insert is smaller than obuf
		if (printed) {
			retval = snprintf(obuf,sizeof(obuf),"%s ",insert );
			printed = SNPRINTED(retval,sizeof(obuf));
		}
		if (nargs) {
			retval = vsnprintf(&(obuf[printed]), sizeof(obuf) - printed, msg, ap); // man page say obuf will always be terminated
			printed += SNPRINTED(retval,sizeof(obuf)-printed);
		} else {
			/* don't do any parsing for format specifiers in the msg -- tshow will
			   also know to do this on the memory side of things */
			retval = snprintf( &(obuf[printed]), sizeof(obuf)-printed, "%s", msg );
			printed += SNPRINTED(retval,sizeof(obuf)-printed);
		}
		if (obuf[printed-1] == '\n')
			obuf[printed-1] = '\0';  // DONE w/ printed (don't need to decrement
		outp = obuf;
	} else {
		if (msg[strlen(msg)-1] == '\n') { // need to copy to remove the trailing nl
			retval = snprintf( obuf, sizeof(obuf), "%s", msg );
			printed = SNPRINTED(retval,sizeof(obuf));
			if (obuf[printed-1] == '\n')
				obuf[printed-1] = '\0';  // DONE w/ printed (don't need to decrement
			outp = obuf;
		} else
			outp = msg;
	}
	ers::LocalContext lc( traceNamLvls_p[TID].name, file, line, function, DEBUG_FORCED );
	std::ostringstream ers_report_impl_out_buffer;
	ers_report_impl_out_buffer << outp;
	switch (lvl)
	{
	case TLVL_INFO:
		ers::info( ers::Message( lc, ers_report_impl_out_buffer.str() )	\
		         BOOST_PP_COMMA_IF( BOOST_PP_NOT( ERS_IS_EMPTY( ERS_EMPTY ERS_EMPTY ) ) ) ERS_EMPTY ); \
		break;
 	case TLVL_LOG:
		ers::log( ers::Message( lc, ers_report_impl_out_buffer.str() )	\
		         BOOST_PP_COMMA_IF( BOOST_PP_NOT( ERS_IS_EMPTY( ERS_EMPTY ERS_EMPTY ) ) ) ERS_EMPTY ); \
		break;
	default:
		lvl -=2 ;
		if ( ers::debug_level() >= lvl )
			ers::debug( ers::Message( lc, ers_report_impl_out_buffer.str() ) \
			           BOOST_PP_COMMA_IF( BOOST_PP_NOT( ERS_IS_EMPTY( ERS_EMPTY lvl ) ) ) lvl ); \
		break;
	}
}

// NOTE: lvl translation occurs
SUPPRESS_NOT_USED_WARN
static void erstrace_user(struct timeval *tvp, int TID, uint8_t lvl, const char* insert, const char* file, int line, const char* function, uint16_t nargs, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	verstrace_user(tvp, TID, lvl-3, insert, file, line, function, nargs, msg, ap);
	va_end(ap);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvarargs"
SUPPRESS_NOT_USED_WARN
static void erstrace_user(struct timeval *tvp, int TID, uint8_t lvl, const char* insert, const char* file, int line, const char* function, uint16_t nargs, const std::string& msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	verstrace_user(tvp, TID, lvl-3, insert, file, line, function, nargs, &msg[0], ap);
	va_end(ap);
}   /* trace */
#pragma GCC diagnostic pop



/*  export TDAQ_ERS_FATAL=erstrace,lstderr
    export TDAQ_ERS_ERROR=erstrace,throttle,lstderr
    export TDAQ_ERS_WARNING=erstrace,throttle,lstderr
 */
#define TRACE_HAS_FUNCTION 0
#include <ers/OutputStream.h>
namespace ers
{
struct erstraceStream : public OutputStream {
        void write( const ers::Issue & issue )
        {
                // can't get "severity" that the stream is associated with.
                // what about the time??
                if TRACE_INIT_CHECK(TRACE_NAME)
                {
                        struct timeval lclTime;
						ers::Severity sev = issue.severity();
                        uint8_t lvl_;
						switch (sev.type) {
						case ers::Debug:       lvl_=5+sev.rank; break;
						case ers::Log:         lvl_=4;          break;
						case ers::Information: lvl_=3;          break;
						case ers::Warning:     lvl_=2;          break;
						case ers::Error:       lvl_=1;          break;
						case ers::Fatal:       lvl_=0;          break;
						}
                        std::chrono::system_clock::time_point tp{issue.ptime()};
                        //auto micros = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch());
                        std::chrono::microseconds micros = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch());
                        lclTime.tv_sec  = micros.count() / 1000000;
                        lclTime.tv_usec = micros.count() % 1000000;
						int traceID = name2TID(trace_path_components(issue.context().file_name(),1));
                        trace(&lclTime, traceID, lvl_, issue.context().line_number(),
# if TRACE_HAS_FUNCTION
                              issue.context().function_name(),
# endif
                              0 TRACE_XTRA_PASSED, issue.message().c_str());
                }
                chained().write( issue );
        }
};
}
ERS_REGISTER_OUTPUT_STREAM( ers::erstraceStream, "erstrace", ERS_EMPTY )    // last param is "param" 




#define LOG0()              
#define LOG1(value)            value,
#define LOG2(value, value1)    value , value1,
#define COMMA_IF_PARENS(...)  ,
#define LPAREN                (
#define EXPAND(...)           __VA_ARGS__
#define CHOOSE(...)           EXPAND(LOG LPAREN \
									 __VA_ARGS__ COMMA_IF_PARENS __VA_ARGS__ COMMA_IF_PARENS __VA_ARGS__ (), \
									 LOG2, impossible, LOG2, LOG1, LOG0, LOG1, ))
#define LOG(a0, a1, a2, a3, a4, a5, arg, ...) arg
#define _tlog_ARG2( a1,a2,...)    tlog_ARG2(a1,a2,__VA_ARGS__)
#define _tlog_ARG3( a1,a2,a3,...) tlog_ARG3(a1,a2,a3,__VA_ARGS__)
//  The following use gnu extension of "##" connecting "," with empty __VA_ARGS__
//  which eats "," when __VA_ARGS__ is empty.
//      pragma GCC diagnostic ignored "-Wvariadic-macros" // doesn't seem to work.
//  Neither does pragma GCC diagnostic ignored "-Wpedantic" // g++ -std=c++11 --help=warnings | egrep -i 'variadic|pedantic'
//  These are last because "pragma GCC system_header" only get undone at the end of the file.
//  This issue is fix in -std=c++2a
#       if (__cplusplus < 201709L)
#               pragma GCC system_header
#       endif
//#undef  TLOG

#undef ERS_INFO
#undef ERS_LOG
#undef ERS_DEBUG
#define ERS_INFO(...)      TRACE_STREAMER(TLVL_INFO+3,					\
										  _tlog_ARG2(not_used, CHOOSE(__VA_ARGS__)(__VA_ARGS__) 0,need_at_least_one), \
										  _tlog_ARG3(not_used, CHOOSE(__VA_ARGS__)(__VA_ARGS__) 0,"",need_at_least_one), \
										  1, SL_FRC(TLVL_INFO) )
#define ERS_LOG(...)       TRACE_STREAMER(TLVL_LOG+3,					\
										  _tlog_ARG2(not_used, CHOOSE(__VA_ARGS__)(__VA_ARGS__) 0,need_at_least_one), \
										  _tlog_ARG3(not_used, CHOOSE(__VA_ARGS__)(__VA_ARGS__) 0,"",need_at_least_one), \
										  1, SL_FRC(TLVL_LOG) )
#define ERS_DEBUG(lvl,...) TRACE_STREAMER(TLVL_DEBUG+lvl+3,			\
										  tlog_ARG2(not_used, ##__VA_ARGS__,0,need_at_least_one), \
										  tlog_ARG3(not_used, ##__VA_ARGS__,0,"",need_at_least_one), \
										  1, SL_FRC(TLVL_DEBUG+lvl) )


#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_LOGGER_HPP_

