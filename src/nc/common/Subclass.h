/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/mpl/int.hpp>

#include <nc/common/CheckedCast.h>

namespace nc {
namespace detail {

template<class Base, class Derived>
class SubclassId {};

}} // namespace nc::detail

#define NC_BASE_CLASS(CLASS, PROPERTY)                                  \
private:                                                                \
    int PROPERTY##_;                                                    \
public:                                                                 \
    int PROPERTY() const { return PROPERTY##_; }                        \
                                                                        \
    template<class T>                                                   \
    bool is() const {                                                   \
        return PROPERTY##_ == nc::detail::SubclassId<CLASS, T>::value;  \
    }                                                                   \
                                                                        \
    template<class T>                                                   \
    T *as() {                                                           \
        if (is<T>()) {                                                  \
            return nc::checked_cast<T *>(this);                         \
        }                                                               \
        return nullptr;                                                 \
    }                                                                   \
                                                                        \
    template<class T>                                                   \
    const T *as() const {                                               \
        if (is<T>()) {                                                  \
            return nc::checked_cast<const T *>(this);                   \
        }                                                               \
        return nullptr;                                                 \
    }                                                                   \
private: 

#define NC_SUBCLASS(BASE, DERIVED, ID)                                  \
    namespace nc { namespace detail {                                   \
        template<>                                                      \
        class SubclassId<BASE, DERIVED>: public boost::mpl::int_<ID> {};\
    }}

/* vim:set et sts=4 sw=4: */
