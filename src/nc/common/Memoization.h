/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <utility>

namespace nc {

template<class Container, class Key, class Function>
decltype((*(Function *)nullptr)()) memoize(Container &container, const Key &key, Function function) {
    auto i = container.find(key);
    if (i != container.end()) {
        return i->second;
    }

    auto result = function();
    container[key] = result;

    return std::move(result);
}

} // namespace nc

/* vim:set et sts=4 sw=4: */
