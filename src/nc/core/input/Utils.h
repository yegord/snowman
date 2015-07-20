/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#pragma once

#include <nc/config.h>

#include <QIODevice>
#include <QString>

#include <nc/common/CheckedCast.h>

namespace nc {
namespace core {
namespace input {

template <class T>
bool read(QIODevice *source, T &obj, std::size_t count = 1) {
    std::size_t size = sizeof(obj) * count;
    return checked_cast<std::size_t>(source->read(reinterpret_cast<char *>(&obj), size)) == size;
}

template <std::size_t size>
QString getAsciizString(const char (&buffer)[size]) {
    return QString::fromLatin1(buffer, qstrnlen(buffer, size));
}

inline QString getAsciizString(const QByteArray &bytes, std::size_t offset = 0) {
    if (offset < checked_cast<std::size_t>(bytes.size())) {
        return QString::fromLatin1(bytes.constData() + offset,
                                   qstrnlen(bytes.constData() + offset, checked_cast<int>(bytes.size() - offset)));
    } else {
        return QString();
    }
}
}}} // namespace nc::core::input

/* vim:set et sts=4 sw=4: */
