#ifndef PURE_CXX_POSIX_PID_HPP
#define PURE_CXX_POSIX_PID_HPP
#pragma once
#if defined(__cplusplus) && (201402L <= __cplusplus)

#include <cerrno>

#include <string>
#include <utility>
#include <sstream>
#include <fstream>
#include <type_traits>

#include <signal.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>

#include "logger.hpp"
#include "context.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include "stdfs.hpp"


namespace pure_cxx_posix {
namespace pid {

    inline static ::std::string path() noexcept(false) {
        auto const &&name = utils::copy(Context::instance().name);
        return stdfs::absolute((name.empty() ? ::std::string{utils::defaultName()} : ::std::move(name)) + ".pid");
    }

    template <class T> inline static auto read(T &&path) noexcept(false) {
        ::std::ifstream stream{::std::forward<T>(path)};
        auto const &&originalMask = stream.exceptions();
        stream.exceptions(stream.failbit | stream.badbit);
        auto &&value = static_cast<::std::decay_t<decltype(::getpid())>>(0); stream >> value;
        stream.exceptions(::std::move(originalMask));
        ::std::string tail; ::std::getline(stream, tail);
        if (! tail.empty()) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"invalid data"});
        stream.close();
        return ::std::make_pair(value, ::std::to_string(::std::move(value)));
    }

    template <class descriptorT, class valueT> inline static auto write(descriptorT &&descriptor, valueT &&value) noexcept(false) {
        ::std::ostringstream stream; stream << ::std::forward<valueT>(value) << ::std::endl; auto string = stream.str();
        auto const &&wr = ::write(descriptor, string.c_str(), string.length());
        if (0 > wr) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"failed to write(): " + utils::errorCodeToString()});
        auto const uwr = static_cast<::std::make_unsigned_t<::std::decay_t<decltype(wr)>>>(::std::move(wr));
        if (uwr != string.length()) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"failed to write(): unknown failure"});
        if (0 != ::fsync(::std::forward<descriptorT>(descriptor)))
            throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"failed to fsync(): " + utils::errorCodeToString()});
    }

    inline static auto lock() noexcept(false) {
        auto const &&pid = ::getpid();
        using Pid = ::std::decay_t<decltype(pid)>;
        Logger::instance() << "My pid is: " << pid;

        auto const &&path = [] () { try { return pid::path(); } catch(...) {
            throw PURE_CXX_POSIX_EXCEPTION_MAKE_FROM_CURRENT(::std::runtime_error{"failed to get pidflie path"});
        } } ();
        Logger::instance() << "My pidfile path is: " << path;

        auto const &&descriptor = ::open(path.c_str(), O_CREAT | O_RDWR, 0666);
        if (0 > descriptor) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"failed to open pidfile: " + utils::errorCodeToString()});

        try {
            if (0 != ::lockf(descriptor, F_TLOCK, 0)) {
                auto const code = errno;
                Logger::instance() << "Hmm, i can't lock pidfile now. Let's try to see who is the ower...";
                if (EAGAIN != code) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{"failed to lockf(): " + utils::errorCodeToString(code)});

                auto const other = [&] () { try { return read(path); } catch(...) {
                    throw PURE_CXX_POSIX_EXCEPTION_MAKE_FROM_CURRENT(::std::runtime_error{"failed to read pidflie"});
                } } ();

                if (0 == other.first) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{
                    "invalid data [" + ::std::to_string(other.first) + "], i want not to kill init =)"
                });
                if (pid == other.first) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{
                    "invalid data [" + ::std::to_string(other.first) + "], i want not to kill my self =)"
                });

                Logger::instance() << "Seems like i found my concurrent with pid = " << other.first << ".";

                auto const path = stdfs::path{"/proc"}.append(::std::move(other.second));
                if (! stdfs::exists(path)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{
                    "i rlly don't know for what i do this test, but path [" + ::std::string{path} + "] doesn't exists"
                });

                if (! stdfs::is_directory(path)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{
                    "i rlly don't know for what i do this test, but this is not directory path [" + ::std::string{path} + "]"
                });

                if (0 < ::kill(other.first, SIGTERM)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{
                    "failed to kill concurrent process: " + utils::errorCodeToString()
                });

                Logger::instance() << "Now i'm trying to lockf() pidfile. It may stuck me... My pid is " << pid << "...";
                if (0 != ::lockf(descriptor, F_LOCK, 0)) throw PURE_CXX_POSIX_EXCEPTION_MAKE(::std::runtime_error{
                    "failed to lockf(): " + utils::errorCodeToString()
                });
                Logger::instance() << "Congratulations! lockf() call success.";
            }

            try { write(descriptor, pid); }
            catch (...) {
                auto const &&backup = utils::copy(errno);
                Logger::instance<Logger::Category::Error>() << "Failed to write pid file, trying to clean it";
                char const zero = 0;
                ::write(descriptor, &zero, 1);
                ::fsync(descriptor);
                errno = ::std::move(backup);
                throw PURE_CXX_POSIX_EXCEPTION_MAKE_FROM_CURRENT(::std::runtime_error{"failed to write pidfile"});
            }
        } catch(...) { ::close(descriptor); throw; }

        using Path = ::std::decay_t<decltype(path)>;
        using Descriptor = ::std::decay_t<decltype(descriptor)>;
        struct Result final { Pid pid = 0; Path path; Descriptor descriptor = -1; };
        return Result{::std::move(pid), ::std::move(path), ::std::move(descriptor)};
    }

} // namespace pid
} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++14 or hight is required"
#endif // defined(__cplusplus) && (201402L <= __cplusplus)
#endif // PURE_CXX_POSIX_PID_HPP
