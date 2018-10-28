#ifndef PURE_CXX_POSIX_EXCEPTION_CONTEXT_FWD_HPP
#define PURE_CXX_POSIX_EXCEPTION_CONTEXT_FWD_HPP
#pragma once
#if defined(__cplusplus) && (201103L <= __cplusplus)

namespace pure_cxx_posix {
namespace exception {
namespace context {

    struct Type;

} // namespace context

    using Context = context::Type;

} // namespace exception
} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++11 or hight is required"
#endif // defined(__cplusplus) && (201103L <= __cplusplus)
#endif // PURE_CXX_POSIX_EXCEPTION_CONTEXT_FWD_HPP
