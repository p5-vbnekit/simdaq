#if defined(__cplusplus) && 201402L <= __cplusplus

#include <cstring>

#include <list>
#include <string>
#include <sstream>
#include <utility>
#include <typeinfo>
#include <numeric>
#include <type_traits>

#include "exception.hpp"


namespace pure_cxx_posix {
namespace exception {
namespace info {

    inline static ::std::string makeString(char const *src) noexcept(false) { if (static_cast<bool>(src)) return src; return {}; }
    inline static auto strerror(int error) noexcept(false) { return makeString(0 == error ? nullptr : ::std::strerror(error)); }

    template <class T> inline static auto makeMessage(char const *what, T &&message, int number = 0) noexcept(false) {
        ::std::ostringstream stream;

        if (number != 0) {
            stream << "Error #" << number;
            auto const text = strerror(number);
            if (! text.empty()) stream << " - " << text;
        }

        auto const details = stream.str();
        ::std::decay_t<decltype(stream)>{}.swap(stream);

        if (message.empty()) {
            if (static_cast<bool>(what) && *what != 0) { stream << what; if (! details.empty()) stream << "[" << details << "]"; }
            else stream << details;
        }

        else {
            stream << ::std::forward<T>(message);
            if (! details.empty()) stream << " [" << details << "]";
        }

        return stream.str();
    }

    template <class T> inline static auto makeTextFromNode(T &&node) noexcept(false) {
        if (static_cast<bool>(node)) return info::text(::std::forward<T>(node));
        return info::makeMessage(nullptr, ::std::string{}, 0);
    }

    Type Type::MakeHelper::implementation(Exception &&exception) noexcept(false) {
        return Info{::std::move(exception.context), "::pure_cxx_posix::Exception", info::makeTextFromNode(::std::move(exception.attachment))};
    }

    Type Type::MakeHelper::implementation(Exception const &exception) noexcept(false) {
        return Info{exception.context, "::pure_cxx_posix::Exception", info::makeTextFromNode(exception.attachment)};
    }

    Type Type::MakeHelper::implementation(::std::exception const &exception) noexcept(false) {
    #if defined(__MINGW32__) && __GNUC__ == 6
        // work around mingw-gcc bug https://sourceforge.net/p/mingw-w64/bugs/626/
        if (typeid(exception).name() == ::std::string{"NSt8ios_base7failureE"}) {
            auto const error = errno;
            if (error != 0) return {"c++ std::ios_base::failure", info::makeMessage(exception.what(), strerror(error), 0)};
        }
    #endif // __MINGW32__ && __GNUC__ == 6
        return Info{"c++ ::std::exception", info::makeMessage(exception.what(), ::std::string{}, 0)};
    }

    Type Type::MakeHelper::implementation(::std::runtime_error const &exception) noexcept(false) {
        return Info{"c++ ::std::runtime_error", info::makeMessage(exception.what(), ::std::string{}, 0)};
    }

    Type Type::MakeHelper::implementation(::std::invalid_argument const &exception) noexcept(false) {
        return Info{"c++ ::std::invalid_argument", info::makeMessage(exception.what(), ::std::string{}, 0)};
    }

    Type Type::MakeHelper::implementation(::std::system_error const &exception) noexcept(false) {
        return Info{"c++ ::std::system_error", info::makeMessage(exception.what(), exception.code().message(), errno)};
    }

    Type Type::MakeHelper::implementation(::std::ios_base::failure const &exception) noexcept(false) {
        return Info{"c++ ::std::ios_base::failure", info::makeMessage(exception.what(), exception.code().message(), errno)};
    }

    Type Type::MakeHelper::implementation(::std::exception_ptr const &exception) noexcept(false) {
        if (static_cast<bool>(exception)) {
            try { ::std::rethrow_exception(exception); }
            catch(::std::ios_base::failure const &exception) { return implementation(exception); }
            catch(::std::system_error const &exception) { return implementation(exception); }
            catch(::std::invalid_argument const &exception) { return implementation(exception); }
            catch(::std::runtime_error const &exception) { return implementation(exception); }
            catch(Exception const &exception) { return implementation(exception); }
            catch(::std::exception const &exception) { return implementation(exception); }
            catch(...) {}
        }

        return implementation();
    }

    template <class containerT, class delimiterT> inline static std::string joinStrings(containerT &&containter, delimiterT &&delimiter) noexcept(false) {
        if (containter.empty()) return {};
        auto const begin = ::std::next(containter.begin());
        return ::std::accumulate(begin, containter.end(), containter.front(), [delimiter = ::std::forward<delimiterT>(delimiter)] (auto &&a, auto &&b) {
            return ::std::forward<decltype(a)>(a) + delimiter + ::std::forward<decltype(b)>(b);
        });
    }

    ::std::string Type::TextHelper::execute(Info &&info) noexcept(false) {
        auto &&text = ::std::move(info.type);
        if (text.empty()) text = "unknown exception";
        auto &&context = ::std::list<::std::string>{};
        if (! info.context.file.empty()) context.push_back("file: " + ::std::move(info.context.file));
        if (! info.context.line.empty()) context.push_back("line: " + ::std::move(info.context.line));
        if (! info.context.function.empty()) context.push_back("function: " + ::std::move(info.context.function));
        if (! context.empty()) text += " at [" + joinStrings(::std::move(context), ", ") + "]";
        return info.message.empty() ? ::std::move(text) : ::std::move(text) + ": " + ::std::move(info.message);
    }

} // namespace info

    char const * Type::what() const noexcept(true) {
        if (static_cast<bool>(attachment)) {
            try { ::std::rethrow_exception(attachment); }
            catch(::std::exception const &e) { auto const *what = e.what(); if (static_cast<bool>(what)) return what; }
            catch(...) {}
        }

        constexpr static auto const * const empty = "";
        return empty;
    }

} // namespace exception
} // namespace pure_cxx_posix

#else
#error "c++14 or hight is required"
#endif // defined(__cplusplus) && 201402L <= __cplusplus
