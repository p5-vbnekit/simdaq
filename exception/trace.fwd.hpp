#ifndef PURE_CXX_POSIX_EXCEPTION_TRACE_FWD_HPP
#define PURE_CXX_POSIX_EXCEPTION_TRACE_FWD_HPP
#pragma once
#if defined(__cplusplus) && (201103L <= __cplusplus)

#include <list>
#include <vector>
#include <cstdint>
#include <utility>

#include "node.hpp"


namespace pure_cxx_posix {
namespace exception {
namespace trace {

    using Item = ::std::pair<Node, ::std::size_t>;

namespace containers {

    using List = ::std::list<Item>;
    using Vector = ::std::vector<Item>;

} // namespace containers
} // namespace trace
} // namespace exception
} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++11 or hight is required"
#endif // defined(__cplusplus) && (201103L <= __cplusplus)
#endif // PURE_CXX_POSIX_EXCEPTION_TRACE_FWD_HPP
