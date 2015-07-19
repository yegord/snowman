/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#pragma once

#include <nc/config.h>

#include <QIODevice>
#include <QString>

namespace nc {
namespace core {
namespace input {

template<class T>
bool read(QIODevice *source, T &obj, std::size_t count = 1) {
    qint64 size = sizeof(obj) * count;
    return source->read(reinterpret_cast<char *>(&obj), size) == size;
}

template<std::size_t size>
QString getString(const char (&buffer)[size]) {
    return QString::fromLatin1(buffer, qstrnlen(buffer, size));
}

inline QString getString(const QByteArray &bytes) {
    return QString::fromLatin1(bytes.constData(), qstrnlen(bytes.constData(), bytes.size()));
}

}}} // namespace nc::core::input

/* vim:set et sts=4 sw=4: */
