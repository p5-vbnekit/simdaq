#ifndef PURE_CXX_POSIX_NOT_OBJECT_HPP
#define PURE_CXX_POSIX_NOT_OBJECT_HPP
#pragma once
#if defined(__cplusplus) && (201103L <= __cplusplus)

namespace pure_cxx_posix {
namespace not_object {

    class Type {
        template <class ... T> Type(T && ...) noexcept(true) = delete;
        template <class T> Type & operator = (T &&) noexcept(true) = delete;
    };

} // namespace not_object

    using NotObject = not_object::Type;

} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++11 or hight is required"
#endif // defined(__cplusplus) && (201103L <= __cplusplus)
#endif // PURE_CXX_POSIX_NOT_OBJECT_HPP
