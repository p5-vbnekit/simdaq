#ifndef PURE_CXX_POSIX_LOGGER_HPP
#define PURE_CXX_POSIX_LOGGER_HPP
#pragma once
#if defined(__cplusplus) && (201402L <= __cplusplus)

#include <mutex>
#include <memory>
#include <string>
#include <utility>
#include <sstream>
#include <iostream>
#include <functional>
#include <type_traits>

#include "context.hpp"
#include "utils.hpp"
#include "not_object.hpp"


namespace pure_cxx_posix {
namespace logger {

    struct Type;
    struct Scope;
    enum class Category { Error, Info };
    using Sink = ::std::function<void(::std::string const &)>;

    auto & instance() noexcept(true);
    template <Category> auto & instance() noexcept(true);

} // namespace logger

    using Logger = logger::Type;

namespace logger {

    struct Type final {
        using Scope = logger::Scope;
        using Category = logger::Category;
        using Sink = logger::Sink;

        Sink sink;
        ::std::mutex mutex;

        auto makeScope() const noexcept(true);
        template <class T> auto operator << (T &&) const noexcept(true);

        template <Category = Category::Info> static auto & instance() noexcept(true);

    private:
        Type() = delete;
        Type(Type &&) = default;
        Type(Type const &) = delete;
        inline explicit Type(Sink &&sink) : sink(::std::move(sink)) {}
        inline explicit Type(Sink const &sink) : sink(sink) {}
        template <class T> Type & operator = (T &&) = delete;
        template <Category> struct SinkMaker;
    };

    template <> struct Type::SinkMaker<Category::Error> final : private NotObject {
        inline static auto execute() noexcept(true) { return [] (auto &&text) {
            ::std::cerr << Context::instance().name << " [ERROR] " << ::std::forward<decltype(text)>(text) << ::std::endl << ::std::flush;
        }; }
    };

    template <Category> struct Type::SinkMaker final : private NotObject {
        inline static auto execute() noexcept(true) { return [] (auto &&text) {
            ::std::clog << Context::instance().name << " [INFO] " << ::std::forward<decltype(text)>(text) << ::std::endl << ::std::flush;
        }; }
    };

    template <Category V> inline auto & Type::instance() noexcept(true) { static Type instance{SinkMaker<V>::execute()}; return instance; }
    template <Category V> inline auto & instance() noexcept(true) { return Type::instance<V>(); }
    inline auto & instance() noexcept(true) { return Type::instance(); }

    struct Scope final {
        using Sink = logger::Sink;

        Sink sink = Type::instance().sink;

        inline auto flush() noexcept(true) {
            if (! static_cast<bool>(mBuffer)) return;
            if (static_cast<bool>(sink)) try {
                auto &&data = mBuffer->str();
                if (data.empty()) return;
                try { auto const l = utils::makeUniqueLock(Type::instance().mutex); utils::unused(l); sink(::std::move(data)); } catch(...) { sink = {}; }
            } catch(...) { mBuffer.reset(nullptr); }
            try { mBuffer->str({}); } catch(...) { mBuffer.reset(nullptr); }
        }

        template <class T> inline auto & operator << (T &&something) noexcept(true) {
            if (static_cast<bool>(mBuffer)) try { *mBuffer << ::std::forward<T>(something); } catch(...) {}
            return *this;
        }

        Scope & operator = (Scope &&) = default;
        Scope & operator = (Scope const &) = default;

        Scope() = default;
        Scope(Scope &&other) = default;
        inline Scope(Scope const &other) : sink{other.sink}, mBuffer{static_cast<bool>(other.mBuffer) ? newBuffer(other.mBuffer->str()) : newBuffer()} {}

        inline explicit Scope(Sink const &sink) noexcept(true) : sink{sink} {}
        inline explicit Scope(Sink &&sink) noexcept(true) : sink{::std::move(sink)} {}

        inline ~Scope() noexcept(true) { flush(); }

    private:
        using Buffer = ::std::ostringstream;
        ::std::unique_ptr<Buffer> mBuffer{newBuffer()};

        template <class ... T> inline static Buffer * newBuffer(T && ... payload) noexcept(true) {
            try { return new Buffer{::std::forward<T>(payload) ...}; } catch(...) {}; return nullptr;
        }
    };

    inline auto Type::makeScope() const noexcept(true) { return Scope{sink}; }

    template <class T> inline auto Type::operator << (T &&something) const noexcept(true) {
        auto &&scope = makeScope(); scope << ::std::forward<T>(something); return ::std::move(scope);
    }

} // namespace logger
} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++14 or hight is required"
#endif // defined(__cplusplus) && (201402L <= __cplusplus)
#endif // PURE_CXX_POSIX_LOGGER_HPP
