#ifndef PURE_CXX_POSIX_EXCEPTION_FWD_HPP
#define PURE_CXX_POSIX_EXCEPTION_FWD_HPP
#pragma once
#ifdef __cplusplus

#include "trace.fwd.hpp"
#include "type.fwd.hpp"
#include "info.fwd.hpp"
#include "context.fwd.hpp"
#include "node.hpp"


namespace pure_cxx_posix {

    namespace exception {}

} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined"
#endif // defined(__cplusplus)
#endif // PURE_CXX_POSIX_EXCEPTION_FWD_HPP
