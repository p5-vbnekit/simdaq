#ifndef PURE_CXX_POSIX_EXCEPTION_NODE_HPP
#define PURE_CXX_POSIX_EXCEPTION_NODE_HPP
#pragma once
#if defined(__cplusplus) && (201103L <= __cplusplus)

#include <exception>


namespace pure_cxx_posix {
namespace exception {
namespace node {

    using Type = ::std::exception_ptr;

} // namespace node

    using Node = node::Type;

} // namespace exception
} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++11 or hight is required"
#endif // defined(__cplusplus) && (201103L <= __cplusplus)
#endif // PURE_CXX_POSIX_EXCEPTION_NODE_HPP
