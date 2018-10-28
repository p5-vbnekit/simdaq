#ifndef PURE_CXX_POSIX_EXCEPTION_TYPE_HPP
#define PURE_CXX_POSIX_EXCEPTION_TYPE_HPP
#pragma once
#if defined(__cplusplus) && (201402L <= __cplusplus)

#include <utility>
#include <exception>
#include <type_traits>

#include "type.fwd.hpp"
#include "node.hpp"
#include "context.hpp"
#include "../not_object.hpp"

#define PURE_CXX_POSIX_EXCEPTION_MAKE(attachment_and_reason...) ::pure_cxx_posix::exception::make(PURE_CXX_POSIX_EXCEPTION_CONTEXT_MAKE(), attachment_and_reason)
#define PURE_CXX_POSIX_EXCEPTION_MAKE_FROM_CURRENT(attachment) PURE_CXX_POSIX_EXCEPTION_MAKE(attachment, ::std::current_exception())


namespace pure_cxx_posix {
namespace exception {

    template <class ... T> static auto make(T && ...) noexcept(false);

    struct Type final : ::std::exception {
        using Node = ::pure_cxx_posix::exception::Node;
        using Context = ::pure_cxx_posix::exception::Context;

        Context context; Node attachment, reason;

        virtual char const * what() const noexcept(true) override final;
        template <class ... T> static auto make(T && ...) noexcept(false);

        Type & operator = (Type &&) = default;
        Type & operator = (Type const &) = default;

        Type() = default;
        Type(Type &&) = default;
        Type(Type const &) = default;

    private:
        // magic constructor condition
        template <class> constexpr static auto mcc() noexcept(true);
        template <class, class, class ...> constexpr static auto mcc() noexcept(true);

        template <class ... T, class = ::std::enable_if_t<mcc<T ...>()>> explicit Type(T && ...) noexcept(false);

        template <class ...> class TypePack;

        template <class T> explicit Type(TypePack<Context> const *, T &&) noexcept(false);
        template <class T> explicit Type(TypePack<Node> const *, T &&something) noexcept(false);

        template <class contextT, class attachmentT> explicit Type(TypePack<Context, Node> const *, contextT &&, attachmentT &&) noexcept(false);
        template <class attachmentT, class contextT> explicit Type(TypePack<Node, Context> const *, attachmentT &&, contextT &&) noexcept(false);
        template <class attachmentT, class reasonT> explicit Type(TypePack<Node, Node> const *, attachmentT &&, reasonT &&) noexcept(false);

        template <class contextT, class attachmentT, class reasonT>
        explicit Type(TypePack<Context, Node, Node> const *, contextT &&, attachmentT &&, reasonT &&) noexcept(false);

        template <class attachmentT, class contextT, class reasonT>
        explicit Type(TypePack<Node, Context, Node> const *, attachmentT &&, contextT &&, reasonT &&) noexcept(false);

        template <class contextT, class attachmentT, class reasonT>
        explicit Type(TypePack<> const *, contextT &&, attachmentT &&, reasonT &&) noexcept(false);

        template <class T> static auto makeFromOther(T &&) noexcept(false);

        template <class T> static auto makeContext(T &&) noexcept(false);
        template <class T> static auto makeAttachment(T &&) noexcept(false);
        template <class T> static auto makeNode(T &&) noexcept(false);

        template <class T> static auto makeAttachment(T &&, void const *) noexcept(false);
        template <class T> static Node makeAttachment(T &&, Node const *) noexcept(false);

        template <class T> static auto makeNode(T &&, void const *) noexcept(false);
        template <class T> static auto makeNode(T &&, Node const *) noexcept(false);

        class TypePackBase : NotObject {};
        template <class ...> class TypePack final : TypePackBase {};

        template <class T> struct MagicCtorTypePackItemTraits;
        template <class T> using MagicCtorTypePackItem = typename MagicCtorTypePackItemTraits<T>::Type;

        template <class ... T> constexpr static auto const * makeMagicCtorTypePack() noexcept(true);
    };


    template <class ... T> inline auto Type::make(T && ... something) noexcept(false) { return Type{::std::forward<T>(something) ...}; }

    template <class T> struct Type::MagicCtorTypePackItemTraits final : private NotObject { using Type = Node; };
    template <> struct Type::MagicCtorTypePackItemTraits<Type::Context> final : private NotObject { using Type = Context; };

    template <class ... T> inline constexpr auto const * Type::makeMagicCtorTypePack() noexcept(true) {
        return static_cast<TypePack<MagicCtorTypePackItem<::std::decay_t<T>> ...> const *>(nullptr);
    }

    template <class ... T, class> inline Type::Type(T && ... arguments) noexcept(false) :
        Type{makeMagicCtorTypePack<T ...>(), ::std::forward<T>(arguments) ...}
    {}

    template <class T> inline Type::Type(TypePack<Context> const *, T &&context) noexcept(false) : context(::std::forward<T>(context)) {}

    template <class T> inline auto Type::makeFromOther(T &&other) noexcept(false) {
        auto &&node = makeNode(::std::forward<T>(other));

        if (static_cast<bool>(node)) {
            try { ::std::rethrow_exception(::std::move(node)); }
            catch (Type const &e) { return e; }
            catch (...) { return Type{makeMagicCtorTypePack<>(), Context{}, ::std::current_exception(), Node{}}; }
        }

        return Type{};
    }

    template <class T> inline Type::Type(TypePack<Node> const *, T &&other) noexcept(false) : Type(makeFromOther(::std::forward<T>(other))) {}

    template <class contextT, class attachmentT> inline
    Type::Type(TypePack<Context, Node> const *, contextT &&context, attachmentT &&attachment) noexcept(false) :
        context{::std::forward<contextT>(context)}, attachment{makeAttachment(::std::forward<attachmentT>(attachment))}
    {}

    template <class attachmentT, class contextT> inline
    Type::Type(TypePack<Node, Context> const *, attachmentT &&attachment, contextT &&context) noexcept(false) :
        context{::std::forward<contextT>(context)}, attachment{makeAttachment(::std::forward<attachmentT>(attachment))}
    {}

    template <class attachmentT, class reasonT> inline
    Type::Type(TypePack<Node, Node> const *, attachmentT &&attachment, reasonT &&reason) noexcept(false) :
        Type{makeFromOther(::std::forward<attachmentT>(attachment))}
    {
        this->reason = makeNode(::std::forward<reasonT>(reason));
    }

    template <class contextT, class attachmentT, class reasonT> inline
    Type::Type(TypePack<Context, Node, Node> const *, contextT &&context, attachmentT &&attachment, reasonT &&reason) noexcept(false) :
        Type{makeMagicCtorTypePack(), ::std::forward<contextT>(context), makeAttachment(::std::forward<attachmentT>(attachment)), makeNode(::std::forward<reasonT>(reason))}
    {}

    template <class attachmentT, class contextT, class reasonT> inline
    Type::Type(TypePack<Node, Context, Node> const *, attachmentT &&attachment, contextT &&context, reasonT &&reason) noexcept(false) :
        Type{makeMagicCtorTypePack(), ::std::forward<contextT>(context), makeAttachment(::std::forward<attachmentT>(attachment)), makeNode(::std::forward<reasonT>(reason))}
    {}

    template <class contextT, class attachmentT, class reasonT> inline
    Type::Type(TypePack<> const *, contextT &&context, attachmentT &&attachment, reasonT &&reason) noexcept(false) :
        context{::std::forward<contextT>(context)}, attachment{::std::forward<attachmentT>(attachment)}, reason{::std::forward<reasonT>(reason)}
    {}

    template <class T> inline auto Type::makeNode(T &&s, void const *) noexcept(false) { return ::std::make_exception_ptr(::std::forward<T>(s)); }
    template <class T> inline auto Type::makeNode(T &&node, Node const *) noexcept(false) { return ::std::forward<T>(node); }
    template <class T> inline auto Type::makeNode(T &&something) noexcept(false) { return makeNode(::std::forward<T>(something), &something); }

    template <class T> inline auto Type::makeContext(T &&something) noexcept(false) {
        try { ::std::rethrow_exception(makeNode(::std::forward<T>(something))); }
        catch (Type const &e) { return e.context; }
        catch (...) {}
        return Context{};
    }

    template <class T> inline Type::Node Type::makeAttachment(T &&node, Node const *) noexcept(false) {
        if (node) {
            try { ::std::rethrow_exception(node); }
            catch (Type const &e) { if (e.context.isEmpty()) return makeAttachment(e.attachment, static_cast<Node const *>(nullptr)); }
            catch (...) {}
        }
        return ::std::forward<T>(node);
    }

    template <class T> inline auto Type::makeAttachment(T &&something, void const *) noexcept(false) {
        return makeAttachment(makeNode(::std::forward<T>(something)), static_cast<Node const *>(nullptr));
    }

    template <class T> inline auto Type::makeAttachment(T &&something) noexcept(false) { return makeAttachment(::std::forward<T>(something), &something); }

    template <class T> inline constexpr auto Type::mcc() noexcept(true) { return ! ::std::is_base_of<Type, ::std::decay_t<T>>::value; }
    template <class T, class, class ...> inline constexpr auto Type::mcc() noexcept(true) {
        return ! ::std::is_convertible<TypePackBase const *, ::std::decay_t<T>>::value;
    }

    template <class ... T> inline static auto make(T && ... something) noexcept(false) { return Type::make(::std::forward<T>(something) ...); }

} // namespace exception
} // namespace pure_cxx_posix

#else
#error "__cplusplus not defined. c++14 or hight is required"
#endif // defined(__cplusplus) && (201402L <= __cplusplus)
#endif // PURE_CXX_POSIX_EXCEPTION_TYPE_HPP
