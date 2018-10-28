#ifndef PURE_CXX_POSIX_EXCEPTION_INFO_HPP
#define PURE_CXX_POSIX_EXCEPTION_INFO_HPP
#pragma once
#if defined(__cplusplus) && (201402L <= __cplusplus)

#include <ios>
#include <string>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <system_error>

#include "../not_object.hpp"

#include "info.fwd.hpp"
#include "type.hpp"
#include "context.hpp"


namespace pure_cxx_posix {
namespace exception {
namespace info {

    struct Type final {
        Context context;
        ::std::string type{"unknown"}, message;

        Type & operator = (Type &&) = default;
        Type & operator = (Type const &) = default;

        Type() = default;
        Type(Type &&) = default;
        Type(Type const &) = default;

        template <class typeT, class messageT>
        inline explicit Type(typeT &&type, messageT &&message) noexcept(false) :
            type{::std::forward<typeT>(type)}, message{::std::forward<messageT>(message)}
        {}

        template <class contextT, class typeT, class messageT>
        inline explicit Type(contextT &&context, typeT &&type, messageT &&message) noexcept(false) :
            context{::std::forward<contextT>(context)}, type{::std::forward<typeT>(type)}, message{::std::forward<messageT>(message)}
        {}

        template <class ... T> static auto text(T && ... something) noexcept(false);
        template <class ... T> static auto make(T && ... something) noexcept(false);

    private:
        struct MakeHelper;
        struct TextHelper;
    };

    template <class ... T> static auto text(T && ... something) noexcept(false);
    template <class ... T> static auto make(T && ... something) noexcept(false);

    struct Type::MakeHelper final : private NotObject {
        template <class ... T> static auto execute(T && ... exception) noexcept(false);

    private:
        using Info = info::Type;
        using Exception = exception::Type;

        static Info implementation() noexcept(false);
        static Info implementation(Info &&info) noexcept(false);
        static Info implementation(Info const &info) noexcept(false);
        static Info implementation(Exception &&exception) noexcept(false);
        static Info implementation(Exception const &exception) noexcept(false);
        static Info implementation(::std::exception const &exception) noexcept(false);
        static Info implementation(::std::runtime_error const &exception) noexcept(false);
        static Info implementation(::std::invalid_argument const &exception) noexcept(false);
        static Info implementation(::std::system_error const &exception) noexcept(false);
        static Info implementation(::std::ios_base::failure const &exception) noexcept(false);
        static Info implementation(::std::exception_ptr const &exception) noexcept(false);

        template <class ... T> class HasImplementation final : NotObject {
            template<class = decltype(implementation(::std::declval<T>() ...))>
            inline constexpr static ::std::true_type test(int const *) noexcept(false);
            inline constexpr static ::std::false_type test(void const *) noexcept(false);
            using TestType = ::std::decay_t<decltype(test(static_cast<int const *>(nullptr)))>;

        public:
            constexpr static auto const value = TestType::value;
        };

        template <class ... T> constexpr static auto hasImplementation() noexcept(false);
        template <class ... T> constexpr static auto hasImplementation(T && ...) noexcept(false);

        template <bool> struct Executor;
    };

    inline Type Type::MakeHelper::implementation() noexcept(false) { return Info{}; }
    inline Type Type::MakeHelper::implementation(Info &&info) noexcept(false) { return ::std::move(info); }
    inline Type Type::MakeHelper::implementation(Info const &info) noexcept(false) { return info; }

    template <class ... T> inline constexpr auto Type::MakeHelper::hasImplementation() noexcept(false) { return HasImplementation<T ...>::value; }
    template <class ... T> inline constexpr auto Type::MakeHelper::hasImplementation(T && ...) noexcept(false) { return HasImplementation<T ...>::value; }

    template <bool> struct Type::MakeHelper::Executor final : private NotObject {
        template <class ... T> inline static auto execute(T && ...) noexcept(false) { return implementation(); }
    };

    template <> struct Type::MakeHelper::Executor<true> final : private NotObject {
        template <class ... T> inline static auto execute(T && ... something) noexcept(false) { return implementation(::std::forward<T>(something) ...); }
    };

    template <class ... T> inline auto Type::MakeHelper::execute(T && ... exception) noexcept(false) {
        return Executor<hasImplementation<T ...>()>::execute(::std::forward<T>(exception) ...);
    }

    template <class ... T> inline auto Type::make(T && ... something) noexcept(false) { return MakeHelper::execute(::std::forward<T>(something) ...); }

    struct Type::TextHelper final : private NotObject {
        static ::std::string execute() noexcept(false);
        static ::std::string execute(Info &&info) noexcept(false);
        static ::std::string execute(Info const &info) noexcept(false);
        template <class T, class = ::std::enable_if_t<! ::std::is_convertible<T, Info>::value>, class = decltype(make(::std::declval<T>()))>
        static auto execute(T &&something) noexcept(false);
    };

    inline ::std::string Type::TextHelper::execute() noexcept(false) { return execute(Info::make()); }
    inline ::std::string Type::TextHelper::execute(Info const &info) noexcept(false) { return execute(Info{info}); }
    template <class T, class, class> inline auto Type::TextHelper::execute(T &&something) noexcept(false) { return execute(make(::std::forward<T>(something))); }

    template <class ... T> inline auto Type::text(T && ... something) noexcept(false) { return TextHelper::execute(::std::forward<T>(something) ...); }

    template <class ... T> inline static auto text(T && ... something) noexcept(false) { return Info::text(::std::forward<T>(something) ...); }
    template <class ... T> inline static auto make(T && ... something) noexcept(false) { return Info::make(::std::forward<T>(something) ...); }

} // namespace info
} // namespace exception
} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++14 or hight is required"
#endif // defined(__cplusplus) && (201402L <= __cplusplus)
#endif // PURE_CXX_POSIX_EXCEPTION_INFO_HPP
