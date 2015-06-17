/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "DefaultDemangler.h"

#include <cstdlib>
#include <memory>

#include <undname/undname.h>

extern "C" {
    /* Provided by libiberty. */
    char *__cxa_demangle(const char *mangled_name, char *output_buffer, size_t *length, int *status);
}

namespace nc {
namespace core {
namespace mangling {

struct FreeDeleter {
    void operator()(void *ptr) const {
        free(ptr);
    }
};

QString DefaultDemangler::demangle(const QString &symbol) const {
    int status;
    auto byteArray = symbol.toLatin1();
    if (auto output = std::unique_ptr<char[], FreeDeleter>(__cxa_demangle(byteArray.constData(), nullptr, nullptr, &status))) {
        return QLatin1String(output.get());
    } else if (auto output = std::unique_ptr<char[], FreeDeleter>(__unDName(nullptr, byteArray.constData(), 0, 0))) {
        return QLatin1String(output.get());
    } else {
        return QString();
    }
}

}}} // namespace nc::core::mangling

/* vim:set et sts=4 sw=4: */
