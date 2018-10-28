#ifndef PURE_CXX_POSIX_EXCEPTION_CONTEXT_HPP
#define PURE_CXX_POSIX_EXCEPTION_CONTEXT_HPP
#pragma once
#if defined(__cplusplus) && (201402L <= __cplusplus)

#include <string>
#include <sstream>
#include <utility>
#include <type_traits>

#include "boost/current_function.hpp"

#include "context.fwd.hpp"

#define PURE_CXX_POSIX_EXCEPTION_CONTEXT_MAKE() ::pure_cxx_posix::exception::context::make(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__)


namespace pure_cxx_posix {
namespace exception {
namespace context {

    template <class ... T> static auto make(T && ...) noexcept(false);

    struct Type final {
        ::std::string file, function, line;

        inline auto isEmpty() const noexcept(false) { return file.empty() && function.empty() && line.empty(); }

        template <class ... T> static auto make(T && ...) noexcept(false);

        Type & operator = (Type &&) = default;
        Type & operator = (Type const &) = default;

        Type() = default;
        Type(Type &&) = default;
        Type(Type const &) = default;

        template <class T, class = ::std::enable_if_t<! ::std::is_base_of<Type, ::std::decay_t<T>>::value>> explicit Type(T &&file) noexcept(false);
        template <class fileT, class functionT> explicit Type(fileT &&, functionT &&) noexcept(false);
        template <class fileT, class functionT, class lineT> explicit Type(fileT &&, functionT &&, lineT &&) noexcept(false);

    private:
        template <class T> static auto makeString(T &&value) noexcept(false);
        template <class T> static auto makeStringFromLine(T &&value) noexcept(false);
        template <class T> static auto makeStringFromLine(T &&value, int *) noexcept(false);
        template <class T> static auto makeStringFromLine(T &&value, void *) noexcept(false);
    };


    template <class T> inline auto Type::makeString(T &&value) noexcept(false) {
        try { return ::std::string{::std::forward<T>(value)}; } catch(...) {}
        return ::std::string{};
    }

    template <class T> inline auto Type::makeStringFromLine(T &&value, int *) noexcept(false) {
        try { ::std::ostringstream stream; stream << ::std::forward<T>(value); return stream.str(); }
        catch(...) {}
        return ::std::string{};
    }

    template <class T> inline auto Type::makeStringFromLine(T &&value, void *) noexcept(false) { return makeString(::std::forward<T>(value)); }

    template <class T> inline auto Type::makeStringFromLine(T &&value) noexcept(false) {
        using Conditional = ::std::conditional_t<::std::is_integral<::std::decay_t<T>>::value, int, void>;
        return makeStringFromLine(::std::forward<T>(value), static_cast<Conditional *>(nullptr));
    }

    template <class T, class> inline Type::Type(T &&file) noexcept(false) : file{makeString(::std::forward<T>(file))} {}

    template <class fileT, class functionT> inline Type::Type(fileT &&file, functionT &&function) noexcept(false) :
        file{makeString(::std::forward<fileT>(file))}, function{makeString(::std::forward<functionT>(function))}
    {}

    template <class fileT, class functionT, class lineT>
    inline Type::Type(fileT &&file, functionT &&function, lineT &&line) noexcept(false) :
        file{makeString(::std::forward<fileT>(file))}, function{makeString(::std::forward<functionT>(function))},
        line{makeStringFromLine(::std::forward<lineT>(line))}
    {}

    template <class ... T> inline auto Type::make(T && ... something) noexcept(false) { return Type(::std::forward<T>(something) ...); }

    template <class ... T> inline static auto make(T && ... something) noexcept(false) { return Type::make(::std::forward<T>(something) ...); }

} // namespace context
} // namespace exception
} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++14 or hight is required"
#endif // defined(__cplusplus) && (201402L <= __cplusplus)
#endif // PURE_CXX_POSIX_EXCEPTION_CONTEXT_HPP
