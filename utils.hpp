#ifndef PURE_CXX_POSIX_UTILS_HPP
#define PURE_CXX_POSIX_UTILS_HPP
#pragma once
#if defined(__cplusplus) && (201402L <= __cplusplus)

#include <cerrno>
#include <cstring>

#include <mutex>
#include <chrono>
#include <string>
#include <iosfwd>
#include <limits>
#include <sstream>
#include <utility>
#include <type_traits>


namespace pure_cxx_posix {
namespace utils {

    template <class periodT = void, class T> inline static auto makeSeconds(T &&value) noexcept(true) {
        return ::std::chrono::duration<::std::decay_t<T>>{::std::forward<T>(value)};
    }

    template <class ... T> inline static auto unused(T && ...) noexcept(true) {}

    template <class mutexT, class ... T> inline static auto makeUniqueLock(mutexT &mutex, T && ... tail) noexcept(true) {
        return ::std::unique_lock<::std::decay_t<mutexT>>{mutex, ::std::forward<T>(tail) ...};
    }

    template <class T> inline static ::std::string errorCodeToString(T &&code) noexcept(false) {
        if (0 != code) {
            auto const * const pointer = ::std::strerror(::std::forward<T>(code));
            if (static_cast<bool>(pointer)) return pointer;
        }

        return "unknown reason";
    }

    inline static auto errorCodeToString() noexcept(false) { return errorCodeToString(errno); }

    inline constexpr static auto const * defaultName() noexcept(true) { return "pcxxpd"; }

    template <class T = int> inline static auto const maxLengthOfNumber() {
        static auto const value = [] () {
            ::std::ostringstream stream;
            stream << ::std::numeric_limits<T>::max();
            stream.seekp(::std::ios_base::beg);
            stream << ::std::numeric_limits<T>::min();
            return stream.str().size();
        } ();

        return value;
    }

    template <class T> inline static ::std::decay_t<T> copy(T &&something) noexcept(true) { return ::std::forward<T>(something); }

} // namespace utils
} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++14 or hight is required"
#endif // defined(__cplusplus) && (201402L <= __cplusplus)
#endif // PURE_CXX_POSIX_UTILS_HPP
