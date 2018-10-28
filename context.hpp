#ifndef PURE_CXX_POSIX_CONTEXT_HPP
#define PURE_CXX_POSIX_CONTEXT_HPP
#pragma once
#if defined(__cplusplus) && (201402L <= __cplusplus)

#include <mutex>
#include <string>
#include <condition_variable>

#include "utils.hpp"


namespace pure_cxx_posix {
namespace context {

    struct Type final {
        ::std::string name = utils::defaultName();
        bool condition = true;
        bool interrupt = false;
        double delay = +2.0e+1;
        ::std::string src = "src", dst = "dst";

        ::std::mutex mutex;
        ::std::condition_variable event;

        inline static auto & instance() noexcept(true) { static Type instance; return instance; }

    private:
        Type() = default;

        template <class ... T> Type(T && ...) = delete;
        template <class T> Type & operator = (T &&) = delete;
    };

    inline static auto & instance() noexcept(true) { return Type::instance(); }

} // namespace context

    using Context = context::Type;

} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++14 or hight is required"
#endif // defined(__cplusplus) && (201402L <= __cplusplus)
#endif // PURE_CXX_POSIX_CONTEXT_HPP
