#ifndef PURE_CXX_POSIX_EXCEPTION_TRACE_HPP
#define PURE_CXX_POSIX_EXCEPTION_TRACE_HPP
#pragma once
#if defined(__cplusplus) && (201402L <= __cplusplus)

#include <cstddef>

#include <list>
#include <vector>
#include <utility>
#include <iterator>
#include <exception>
#include <type_traits>
#include <initializer_list>

#include "trace.fwd.hpp"
#include "type.hpp"
#include "node.hpp"


namespace pure_cxx_posix {
namespace exception {
namespace trace {

    template <template <class> class containerC, class T> static auto make(T &&) noexcept(true);
    template <class containerT = containers::List, class T> static auto make(T &&) noexcept(true);

    struct MakeHelper final : private NotObject {
        template <class containerT, class T> static auto execute(T &&) noexcept(true);
        template <template <class> class containerC, class T> static auto execute(T &&) noexcept(true);

    private:
        template <class T> static auto execute(T &&, containers::Vector const *) noexcept(true);
        template <class T> static auto execute(T &&, containers::List const *) noexcept(true);
        template <class T> static auto execute(T &&, void const *) = delete;

        template <class T> static auto extract(T &&) noexcept(true);
        template <class T> static auto extract(T &&, void const *) noexcept(true);
        template <class T> static auto extract(T &&, exception::Type const *) noexcept(true);
        template <class T> static containers::List extract(T &&, Node const *) noexcept(true);
    };

    template <class T> inline auto MakeHelper::extract(T &&something, void const *) noexcept(true) {
        return containers::List{{::std::make_exception_ptr(::std::forward<T>(something)), 0}};
    }

    template <class T> inline auto MakeHelper::extract(T &&object, exception::Type const *) noexcept(true) {
        if (object.reason) {
            auto &&list = extract(object.reason, static_cast<Node const *>(nullptr));
            list.push_back({::std::make_exception_ptr(::std::forward<T>(object)), list.back().second + 1});
            return ::std::move(list);
        }

        return containers::List{{::std::make_exception_ptr(::std::forward<T>(object)), 0}};
    }

    template <class T> inline containers::List MakeHelper::extract(T &&node, Node const *) noexcept(true) {
        try { ::std::rethrow_exception(node); }
        catch(exception::Type const &object) {
            if (object.reason) {
                auto &&list = extract(object.reason, static_cast<Node const *>(nullptr));
                list.push_back({::std::forward<T>(node), list.back().second + 1});
                return ::std::move(list);
            }
        }
        catch(...) {}
        return containers::List{{::std::forward<T>(node), 0}};
    }

    template <class T> inline auto MakeHelper::extract(T &&something) noexcept(true) { return extract(::std::forward<T>(something), &something); }

    template <class T> inline auto MakeHelper::execute(T &&something, containers::Vector const *) noexcept(true) {
        auto &&pair = extract(::std::forward<T>(something));
        return containers::Vector(::std::make_move_iterator(::std::begin(pair.second)), ::std::make_move_iterator(::std::end(pair.second)));
    }

    template <class T> inline auto MakeHelper::execute(T &&something, containers::List const *) noexcept(true) {
        return extract(::std::forward<T>(something));
    }

    template <class containerT, class T> inline auto MakeHelper::execute(T &&something) noexcept(true) {
        return execute(::std::forward<T>(something), static_cast<containerT const *>(nullptr));
    }

    template <template <class> class containerC, class T> inline auto MakeHelper::execute(T &&something) noexcept(true) {
        return execute(::std::forward<T>(something), static_cast<containerC<exception::Type> const *>(nullptr));
    }

    template <class containerT, class T> inline static auto make(T &&something) noexcept(true) {
        return MakeHelper::execute<containerT>(::std::forward<T>(something));
    }

    template <template <class> class containerC, class T> inline static auto make(T &&something) noexcept(true) {
        return MakeHelper::execute<containerC>(::std::forward<T>(something));
    }

} // namespace trace
} // namespace exception
} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++14 or hight is required"
#endif // defined(__cplusplus) && (201402L <= __cplusplus)
#endif // PURE_CXX_POSIX_EXCEPTION_TRACE_HPP
