#include <tinfra/option.h>
#include <tinfra/lex.h>
#include <tinfra/logger.h> // for log_info, log_fail
#include <tinfra/file.h>
#include <tinfra/exeinfo.h>
#include <tinfra/path.h>
#include <tinfra/fmt.h>
#include <tinfra/os_common.h> // errno_to_string

#include <unistd.h>   // for fork, setsid, close
#include <stdlib.h>   // for exit
#include <sys/types.h>
#include <sys/stat.h> // for umask
#include <fcntl.h>
#include <cerrno>
#include <syslog.h>

//
// externs
//

extern void maybe_initialize_syslog_logging();
extern void maybe_daemonize();

tinfra::option_switch       opt_use_syslog('s', "syslog","log to syslog, default app_name, user");
tinfra::option<std::string> opt_syslog_tag("","syslog-tag","application tag used when logging to syslog");
tinfra::option<std::string> opt_syslog_facility("user","syslog-facility","facility used when logging to syslog (default user)");

tinfra::option_switch       opt_daemonize("daemon","run as daemon in background");
tinfra::option<std::string> opt_pidfile("","pid-file","save process pid in file");

//
// logging
//
// warning, this doesn't work in SIGNAL HANDLERS!
//

class syslog_log_handler: public tinfra::log_handler {
private:
    virtual void log(tinfra::log_record const& record)
    {
        const int level = (
            (record.level == tinfra::LL_FATAL ) ? LOG_ALERT :
            (record.level == tinfra::LL_FAIL ) ? LOG_CRIT :
            (record.level == tinfra::LL_ERROR ) ? LOG_ERR :
            (record.level == tinfra::LL_NOTICE ) ? LOG_NOTICE :
            (record.level == tinfra::LL_WARNING ) ? LOG_WARNING :
            (record.level == tinfra::LL_INFO ) ? LOG_INFO :
            (record.level == tinfra::LL_TRACE) ? LOG_DEBUG : LOG_ERR
        );

        // at least on OSX syslog doesn't work in signal handlers !
        ::syslog(level, "%.*s", (int)record.message.size(), record.message.data());
    }
};

static syslog_log_handler the_syslog_handler;

std::string get_default_tag() {
    return tinfra::path::basename(tinfra::path::remove_extension(tinfra::get_exepath()));
}

void maybe_initialize_syslog_logging()
{
    if( opt_use_syslog.enabled() ) {
        const std::string facility_value = opt_syslog_facility.value();
        const int facility = (
            (facility_value == "" || facility_value == "user" ) ? LOG_USER :
            (facility_value == "auth" )     ? LOG_AUTH :
            (facility_value == "authpriv" ) ? LOG_AUTHPRIV :
            (facility_value == "cron" )     ? LOG_CRON :
            (facility_value == "daemon" )   ? LOG_DAEMON :
            // (facility_value == "security" ) ? LOG_SECURITY :
            (facility_value == "local0" )   ? LOG_LOCAL0 :
            (facility_value == "local1" )   ? LOG_LOCAL1 :
            (facility_value == "local2" )   ? LOG_LOCAL2 :
            (facility_value == "local3" )   ? LOG_LOCAL3 :
            (facility_value == "local4" )   ? LOG_LOCAL4 :
            (facility_value == "local5" )   ? LOG_LOCAL5 :
            (facility_value == "local6" )   ? LOG_LOCAL6 :
            (facility_value == "local7" )   ? LOG_LOCAL7 : LOG_USER
        ) ;

        const std::string app_name = opt_syslog_tag.accepted() ? opt_syslog_tag.value() : get_default_tag();

        const int log_options = LOG_PID | LOG_NDELAY;
        ::openlog(app_name.c_str(), log_options, facility);

        tinfra::log_handler::set_default(&the_syslog_handler);
    }
}

//
// daemon
//
void become_daemon()
{
    int r;

    r = ::fork();
    if( r > 0 ) {
        // parent is not interested
        tinfra::log_info(tinfra::tsprintf("entered daemon mode, pid=%i", r));
        exit(EXIT_SUCCESS);
    }
    if( r < 0 ) {
        tinfra::log_error(tinfra::tsprintf("unable to fork: %s", tinfra::errno_to_string(errno)));
        exit(EXIT_FAILURE);
    }

    if( chdir("/") < 0 ) {
        tinfra::log_error(tinfra::tsprintf("unable to chdir to '/': %s (ignoring)", tinfra::errno_to_string(errno)));
    }

    umask(007);

    {
        r = setsid();
        if (r < 0) {
            tinfra::log_error(tinfra::tsprintf("unable to set sessionid (setsid): %s", tinfra::errno_to_string(errno)));
            exit(EXIT_FAILURE);
        }
    }
}

void maybe_daemonize()
{
    using tinfra::fmt;
    using tinfra::log_info;
    using tinfra::log_fail;

    if( opt_daemonize.enabled() ) {
        become_daemon();
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    if( opt_pidfile.accepted() && opt_pidfile.value() != "" ) {

        const int pid = getpid();
        const std::string content = tinfra::to_string(pid);

        tinfra::write_file(opt_pidfile.value(), content);

        log_info(fmt("saved pid to: %s") % opt_pidfile.value());
    }
}

