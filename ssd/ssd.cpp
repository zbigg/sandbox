///
/// ssd - small {starter, system, supervision} daemon 
///
/// small, personal and (distributed future) systemd/upstart/launchd
/// daemon
///
/// usage:
///    ssd                             -- start local daemon
///    ssd status                      -- show services status
///    ssd enable NAME command         -- start service NAME
///    ssd disable NAME command        -- add persitent service
///

#include <tinfra/tstring.h>
#include <tinfra/fmt.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>

#include <tinfra/subprocess.h> // we actually manage subprocesses 
                               // of this type

#include <tinfra/cli.h>
#include <tinfra/logger.h>
#include <tinfra/os_common.h>
#include <signal.h>

template <typename T>
class event_emitter {
    using callback_t = std::function<void(T const&)>;
    using listeners_t = std::vector<callback_t>;

    listeners_t listeners;

public:
    event_emitter& add_listener(callback_t callback) {
        listeners.push_back(std::move(callback));
        return *this;
    }

    event_emitter& remove_listener(callback_t callback) {
        auto it = std::find(listeners.begin(), listeners.end(), callback);
        if( it != listeners.end() ) {
            listeners.erase(it);
        }
        return *this;
    }

    void emit(T const& value) {
        for(auto const& c: this->listeners ) {
            c(value);
        }
    }
};

/*
template <typename T>
class async_event_emitter: public event_emitter {
    void emit(T value) {
        for(auto c: this->listeners ) {
            async([c,value] {
                c(value);
            });
        }
    }
};
*/


///
/// generic stuff
///
using tinfra::tstring;
using tinfra::log_info;
using tinfra::tsprintf;

template <typename... T>
void log(tinfra::tstring const& fmt, T&&... args) {
    tinfra::log_info(tinfra::tsprintf(fmt, args...));
}

//
// timer_manager
//
/*
class timer_manager {
    using tinfra::time_stamp;
    std::priority_queue<time_stamp> times;
};
*/

using std::shared_ptr;
using std::string;
using std::vector;
using std::map;
using std::unique_ptr;

struct process_exit_result {
    int exit_status;
    int signal_number;
};

struct runtime_process_entry {
    int pid = -1;
    unique_ptr<tinfra::subprocess> sp;

    bool running = false;
    bool terminate_sent = false;

    //unique_ptr<process_exit_result> exit_result;
        // valid for exited processes

    // event_emitter<process_exit_result> exit_event;
};


struct process_entry {
    string         name;
    vector<string> command;
    string         working_dir;

    shared_ptr<runtime_process_entry> re;
};

struct runtime_state {
    map<int, shared_ptr<runtime_process_entry>> running;

    vector<process_entry> entries;
};

void process_exited_processes(runtime_state& rts)
{
    while( true ) {
        int status;
        int pid = ::waitpid(-1, &status, WNOHANG);
        if( pid == 0 || pid == -1 ) {
            // TBD: error in case of -1
            return;
        }
        auto pei = rts.running.find(pid);
        if( pei == rts.running.end() ) {
            log("ignoring unknown process exit, pid=%s", pid);
            continue;
        }
        process_exit_result per;
        per.exit_status = 1;
        per.signal_number = 0;

        if(  WIFSIGNALED(status) ) {
            per.signal_number = WTERMSIG(status);
        }
        if( WIFEXITED(status) ) {
            per.exit_status = WEXITSTATUS(status);
        }
        log("exited, pid=%s, ec=%s, s=%s", pid, per.exit_status, per.signal_number);

        // pei->second->exit_event.emit(per);
        // pei->second->exit_result.reset(new process_exit_result(per));
        pei->second->running = false;

        rts.running.erase(pei);
    }
}

void start_outstanding_processes(runtime_state& rts)
{
    for(auto& pe: rts.entries ) {
        if( pe.re && pe.re->running ) {
            continue;
        }

        log("[%s] not running, checking when to start", pe.name);
        log("[%s] starting", pe.name);
        // TBD, calculate start jitter & restart interval
        pe.re.reset( new runtime_process_entry );
        pe.re->running = false;
        pe.re->sp.reset( tinfra::create_subprocess() );
        if( pe.working_dir != "" ) {
            pe.re->sp->set_working_dir(pe.working_dir);
        }
        pe.re->sp->start(pe.command);
        pe.re->pid = pe.re->sp->get_native_handle();
        rts.running[pe.re->pid] = pe.re;
        log("[%s] started, pid=%s", pe.name, pe.re->pid);
        pe.re->running = true;
    }
}

void graceful_terminate_processes(runtime_state& rts)
{
    for(auto& rpei: rts.running ) {
        runtime_process_entry& rpe = *(rpei.second.get());
        if( ! rpe.terminate_sent ) {
            log("graceful_terminate_processes, terminating, pid=%s", rpe.pid);
            rpe.terminate_sent = true;
            rpe.sp->terminate();
        }
        // TBD, after some timeout send kill!
    }
}

//
// SIGCHLD
//

sig_atomic_t some_children_exited = 0;
static void sigchld_handler(int, siginfo_t *, void *)
{
     std::cerr << "sigchld\n";
   // log("sigchld");
    some_children_exited = 1;
}

void install_sigchld_handler()
{
    struct sigaction act;
    memset (&act, '\0', sizeof(act));
    act.sa_sigaction = sigchld_handler;
    act.sa_flags   = SA_SIGINFO;
    int r =  sigaction(SIGCHLD, &act, (struct sigaction*)0);
    if( r  == -1) {
        TINFRA_LOG_ERROR(tinfra::tsprintf("unable to install signal(%i) handler: %s", SIGCHLD, tinfra::errno_to_string(errno)));
    }
}

//
// SIGTERM & SIGINT
//

sig_atomic_t terminate_requested = 0;
static void sigterm_handler(int signo, siginfo_t *, void *)
{
    // log("sigterm (%s)", signo);
    std::cerr << "sigerm\n";
    terminate_requested = 1;
}

void install_sigterm_handler()
{
    struct sigaction act;
    memset (&act, '\0', sizeof(act));
    act.sa_sigaction = sigterm_handler;
    act.sa_flags   = SA_SIGINFO;
    int r =  sigaction(SIGTERM, &act, (struct sigaction*)0);
    if( r  == -1) {
        TINFRA_LOG_ERROR(tinfra::tsprintf("unable to install signal(%i) handler: %s", SIGTERM, tinfra::errno_to_string(errno)));
    }
    int r2 =  sigaction(SIGINT, &act, (struct sigaction*)0);
    if( r2  == -1) {
        TINFRA_LOG_ERROR(tinfra::tsprintf("unable to install signal(%i) handler: %s", SIGINT, tinfra::errno_to_string(errno)));
    }
}

//
// loop
//

void loop(runtime_state& rts)
{
    sigset_t mask;
    sigset_t old_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, &old_mask);

    start_outstanding_processes(rts);
    while( true ) {
        // maybe terminate ?
        if( terminate_requested ) {
            if( rts.running.size() == 0 ) {
                break;
            }
            graceful_terminate_processes(rts);
        }

        // maybe start unstarted things
        if( !terminate_requested ) {
            start_outstanding_processes(rts);
        }

        // process exit signals
        if ( some_children_exited ) {
            some_children_exited = 0;
            process_exited_processes(rts);
            continue;
        }

        // wait for next events
        sigsuspend(&old_mask);
    }
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    log("all done, exiting");
}

extern void maybe_initialize_syslog_logging();
extern void maybe_daemonize();


int ssd_main(tstring const& program_name, std::vector<tinfra::tstring>& args)
{
    maybe_initialize_syslog_logging();
    maybe_daemonize();
    runtime_state rts;

    // TBD, read config
    {
        process_entry pe;
        pe.name = "p1";
        pe.command = vector<string> { "sleep", "5" };
        rts.entries.push_back(pe);
    }
    {
        process_entry pe;
        pe.name = "p2";
        pe.command = vector<string> { "sleep", "2" };
        rts.entries.push_back(pe);
    }
    {
        process_entry pe;
        pe.name = "p3";
        pe.command = vector<string> { "sleep", "3" };
        rts.entries.push_back(pe);
    }
    install_sigchld_handler();
    install_sigterm_handler();
    loop(rts);
    return 0;
}

int main(int argc, char** argv)
{
    return tinfra::cli_main(argc, argv, ssd_main);
}
