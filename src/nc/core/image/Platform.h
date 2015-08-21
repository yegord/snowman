/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QString>

namespace nc { namespace core {

namespace arch {
    class Architecture;
}

namespace image {

/**
 * A modest clone of llvm::Triple, describes the target platform of an executable.
 */
class Platform {
public:
    enum OperatingSystem {
        UnknownOS,
        Windows
    };

    Platform();

    /**
     * Sets the architecture of the platform.
     *
     * \param architecture Pointer to the architecture. Can be NULL.
     */
    void setArchitecture(const arch::Architecture *architecture);

    /**
     * Sets the architecture of the platform to the architecture
     * with the given name. If no architecture with this name
     * exists, sets it to nullptr.
     *
     * \param name Name of the architecture.
     */
    void setArchitecture(const QString &name);

    /**
     * \return Pointer to the architecture. Can be nullptr.
     */
    const arch::Architecture *architecture() const { return architecture_; }

    /**
     * Sets the operating system of the platform.
     *
     * \param operatingSystem Operating system.
     */
    void setOperatingSystem(OperatingSystem operatingSystem);

    /**
     * \return The operating system of the platform.
     */
    OperatingSystem operatingSystem() const { return operatingSystem_; }

private:
    const arch::Architecture *architecture_;
    OperatingSystem operatingSystem_;
};

}}} // namespace nc::core::image

/* vim:set et sts=4 sw=4: */
