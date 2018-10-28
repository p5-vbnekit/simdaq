#if defined(__cplusplus) && 201402L <= __cplusplus

#include <cerrno>
#include <cstring>
#include <cstdint>

#include <mutex>
#include <memory>
#include <thread>
#include <chrono>
#include <ostream>
#include <sstream>
#include <utility>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <condition_variable>

#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>

#include "pid.hpp"
#include "logger.hpp"
#include "context.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include "stdfs.hpp"


namespace pure_cxx_posix {

    template <class streamT, class exceptionT> inline static auto printException(streamT &&stream, exceptionT &&exception) noexcept(false) {
        auto const bt = exception::trace::make(::std::forward<exceptionT>(exception));
        if (bt.empty()) stream << "unknown exception caught";
        else if (0 < bt.back().second) {
            stream << "exception caught, backtrace [" << bt.rbegin()->second + 1 << "]:"; stream.flush();
            for (auto const &item : bt) { stream << "  #" << item.second << ": " << exception::info::text(item.first); stream.flush(); }
        }
        else stream << "exception caught: " << exception::info::text(bt.front().first);
    }

    template <class T> inline static auto printException(T &&stream) noexcept(false) {
        return printException(::std::forward<T>(stream), ::std::current_exception());
    }

    inline static auto printException() noexcept(false) {
        return printException(Logger::instance<Logger::Category::Error>().makeScope(), ::std::current_exception());
    }

namespace config {

    inline static ::std::string path() noexcept(false) {
        auto const &&name = utils::copy(Context::instance().name);
        return stdfs::absolute((name.empty() ? ::std::string{utils::defaultName()} : ::std::move(name)) + ".conf");
    }

    inline static auto const read() noexcept(false) {
        auto stream = ::std::ifstream{path(), ::std::ios_base::in};
        stream.exceptions(stream.failbit | stream.badbit);
        using Path = ::std::string; auto &&src = Path{}; auto &&dst = Path{};
        using Delay = double; auto &&delay = ::std::numeric_limits<Delay>::quiet_NaN();
        stream >> src; stream >> dst; stream >> delay;
        if (src.empty()) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"invalid data"});
        if (dst.empty()) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"invalid data"});
        if (! (+1.0e+0 < delay)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"invalid data"});
        stream.close();
        struct Result final { Path src, dst; Delay delay; };
        return Result{::std::move(src), ::std::move(dst), ::std::move(delay)};
    }

} // namespace config

namespace daemon {

    template <class T> inline static auto atomicWithContext(T &&action) {
        auto &context = Context::instance();
        auto const lock = utils::makeUniqueLock(context.mutex); utils::unused(lock); return action(context);
    }

    inline static auto signalHandler(int id) noexcept(true) {
        switch (id) {
        default:
            Logger::instance() << "Unknown signal [" << id << "] caught";
            break;

        case SIGTERM:
            Logger::instance() << "SIGTERM caught";
            return atomicWithContext([] (auto &&context) { context.condition = false; context.event.notify_all(); });

        case SIGHUP:
            Logger::instance() << "SIGHUP caught";
            return atomicWithContext([] (auto &&context) { context.interrupt = true; context.event.notify_all(); });
        }
    }

    template <class srcT, class dstT> inline static auto moveFiles(srcT &&src, dstT &&dst) noexcept(false) {
        if (src.empty()) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::invalid_argument{"source directory path is empty"});
        if (dst.empty()) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::invalid_argument{"destination directory path is empty"});
        Logger::instance() << "Moving files from " << src << " to " << dst << "...";

        auto const srcPath = stdfs::path{::std::forward<srcT>(src)};
        if (! stdfs::exists(srcPath)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::invalid_argument{"source directory is not exists"});
        if (! stdfs::is_directory(srcPath)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::invalid_argument{"source is not directory"});

        auto const dstPath = stdfs::path{::std::forward<dstT>(dst)};
        if (! stdfs::exists(dstPath)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::invalid_argument{"destination directory is not exists"});
        if (! stdfs::is_directory(dstPath)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::invalid_argument{"destination is not directory"});

        auto flag = true;
        auto &context = Context::instance();

        while(true) {
            context.mutex.lock();
            flag = (! context.condition) || context.interrupt;
            context.mutex.unlock();
            if (flag) break;
            flag = true;

            for (auto const &item : stdfs::directory_iterator{srcPath}) try {
                if (! stdfs::is_regular_file(item)) continue;
                auto const relative = stdfs::path{item}.filename();
                if (relative.empty()) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::invalid_argument{"empty relative path"});
                auto const src = stdfs::absolute(relative, srcPath);
                auto const dst = stdfs::absolute(relative, dstPath);
                Logger::instance() << "Moving " << src << " to " << dst << "...";
                try { errno = 0; stdfs::copy_file(src, dst, stdfs::copy_options::overwrite_existing); stdfs::remove(src); }
                catch(...) { throw PURE_CXX_POSIX_EXCEPTION_MAKE_FROM_CURRENT(::std::runtime_error{
                    "failed to move src " + ::std::string{src} + " to " + ::std::string{dst}
                }); };
                flag = false; break;
            } catch (...) { printException(); }

            if (flag) break;
        }
    }

    inline static auto applyConfig() noexcept(true) {
        auto &logger = Logger::instance();
        auto &context = Context::instance();
        try {
            logger << "Reading config now...";
            auto const &&data = config::read();
            context.src = ::std::move(data.src);
            context.dst = ::std::move(data.dst);
            context.delay = ::std::move(data.delay);
        } catch(...) { printException(Logger::instance<Logger::Category::Error>().makeScope() << "Failed to load config: "); }

        logger << "My current settings: src = " << context.src << ", dst = " << context.dst << ", delay = " << context.delay;
    }

    inline static auto loop() noexcept(false) {
        Logger::instance() << "My config path is: " << config::path();

        applyConfig();
        auto &context = Context::instance();
        auto lock = utils::makeUniqueLock(context.mutex);

        while (context.condition) {
            auto const moment = ::std::chrono::steady_clock::now() + utils::makeSeconds(context.delay);
            while (context.condition) {
                context.event.wait_until(lock, moment);
                if (context.interrupt) { context.interrupt = false; applyConfig(); break; }
                if (moment <= ::std::chrono::steady_clock::now()) {
                    auto pair = ::std::make_pair(context.src, context.dst);
                    lock.unlock();
                    try { moveFiles(::std::move(pair.first), ::std::move(pair.second)); } catch(...) { printException(); }
                    lock.lock();
                    break;
                }
            }
        }
    }

    inline static auto main() noexcept(false) {
        Logger::instance<Logger::Category::Info>().sink = [] (auto &&message) { ::syslog(LOG_INFO, "[INFO] %s", message.c_str()); };
        Logger::instance<Logger::Category::Error>().sink = [] (auto &&message) { ::syslog(LOG_ERR, "[ERROR] %s", message.c_str()); };
        Logger::instance() << "Hello, world!" << " This is child process";

        if (0 != ::close(STDIN_FILENO)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"STDIN close() failure: " + utils::errorCodeToString()});
        if (0 != ::close(STDOUT_FILENO)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"STDOUT close() failure: " + utils::errorCodeToString()});
        if (0 != ::close(STDERR_FILENO)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"STDERR close() failure: " + utils::errorCodeToString()});
        Logger::instance() << "STDIN, STDOUT, STDERR closed";

        if (SIG_ERR == ::signal(SIGHUP, signalHandler))
            throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"SIGHUP handler activation failure: " + utils::errorCodeToString()});
        if (SIG_ERR == ::signal(SIGTERM, signalHandler))
            throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"SIGTERM handler activation failure: " + utils::errorCodeToString()});
        Logger::instance() << "SIGHUP, SIGTERM handler activated";

        auto const lock = [] () { try { return pid::lock(); } catch(...) {
            throw PURE_CXX_POSIX_EXCEPTION_MAKE_FROM_CURRENT(::std::runtime_error{"failed to lock pidflie"});
        } } ();

        Logger::instance() << "Hello, Hell! I'm daemon now. =p";
        Logger::instance() << "My pid: " << lock.pid;
        Logger::instance() << "My pidfile: " << lock.path;

        try { loop(); } catch(...) { throw PURE_CXX_POSIX_EXCEPTION_MAKE_FROM_CURRENT(::std::runtime_error{"loop failed"}); }
    }

} // namespace daemon

    inline static auto main() noexcept(false) {
        auto const pid = ::fork();

        if (0 < pid) Logger::instance() << "Hello, world! This is parent process.";
        else if (0 == pid) daemon::main();
        else throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"failed to fork: " + utils::errorCodeToString()});

        Logger::instance() << "Bye, world!";
        return EXIT_SUCCESS;
    }

} // namespace pure_cxx_posix

int main(int argc, char **argv) {
    namespace pcxxp = ::pure_cxx_posix;

    pcxxp::Logger::instance() << "pure c++ and posix daemon example by vbnekit@gmail.com";

    auto &name = pcxxp::Context::instance().name;
    if (0 < argc && static_cast<bool>(argv[0]) && (0 != argv[0][0])) name = pcxxp::stdfs::path{argv[0]}.filename();

    try { return pcxxp::main(); } catch(...) { pcxxp::printException(); }
    auto const code = errno;

    pcxxp::Logger::instance<pcxxp::Logger::Category::Error>() << "I'm failed. ='(";
    pcxxp::Logger::instance<pcxxp::Logger::Category::Info>() << "Bye, world!";
    return 0 == code ? EXIT_FAILURE : code;
}

#else
#error "c++14 or hight is required"
#endif // defined(__cplusplus) && 201402L <= __cplusplus
