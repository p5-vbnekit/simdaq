#ifndef PURE_CXX_POSIX_STDFS_HPP
#define PURE_CXX_POSIX_STDFS_HPP
#pragma once
#ifdef __cplusplus

#undef STDFS_EXPERIMENTAL
#if 201703L <= __cplusplus
#define STDFS_EXPERIMENTAL 0
#elif 201402L <= __cplusplus
#define STDFS_EXPERIMENTAL 1
#else
#error "c++14 or hight is required"
#endif

#if (0 != STDFS_EXPERIMENTAL)
#include <experimental/filesystem>
#else
#include <filesystem>
#endif // STDFS_EXPERIMENTAL


namespace pure_cxx_posix {

#if (0 != STDFS_EXPERIMENTAL)
    namespace stdfs = ::std::experimental::filesystem;
#else
    namespace stdfs = ::std::filesystem;
#endif // 0 != STDFS_EXPERIMENTAL

} // namespace pure_cxx_posix

#else
#error "c++14 or hight is required"
#endif // __cplusplus
#endif // PURE_CXX_POSIX_STDFS_HPP
