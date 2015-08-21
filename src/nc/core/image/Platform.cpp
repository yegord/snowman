/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Platform.h"

#include <nc/core/arch/ArchitectureRepository.h>

namespace nc { namespace core { namespace image {

Platform::Platform():
    architecture_(nullptr),
    operatingSystem_(UnknownOS)
{}

void Platform::setArchitecture(const QString &name) {
    setArchitecture(arch::ArchitectureRepository::instance()->getArchitecture(name));
}

void Platform::setArchitecture(const arch::Architecture *architecture) {
    architecture_ = architecture;
}

void Platform::setOperatingSystem(OperatingSystem operatingSystem) {
    operatingSystem_ = operatingSystem;
}

}}} // namespace nc::core::image

/* vim:set et sts=4 sw=4: */
