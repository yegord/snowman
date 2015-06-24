/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/optional.hpp>

#include <QString>

namespace nc {

/**
 * Converts a string representation of an integer to an integer.
 *
 * \param s A string.
 * \param base Conversion base. 0 for C-style conversions understanding prefixes 0x and 0.
 * \tparam T Integer type to which to convert.
 *
 * \return The resulting integer on success, boost::none on failure.
 */
template<class T>
boost::optional<T>
stringToInt(const QString &s, int base = 10);

} // namespace nc

/* vim:set et sts=4 sw=4: */
