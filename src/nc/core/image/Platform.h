/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QString>

#include <nc/common/Types.h>

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
        Windows,
        DOS
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

    /**
     * Sets size of int in bits for target platform.
     *
     * \param[in] intSize Size.
     */
    void setIntSize(SmallBitSize intSize) { intSize_ = intSize; }

    /**
     * \return Size of int in bits for target platform.
     */
    SmallBitSize intSize() const { return intSize_; }

private:
    const arch::Architecture *architecture_;
    OperatingSystem operatingSystem_;
    SmallBitSize intSize_;
};

}}} // namespace nc::core::image

/* vim:set et sts=4 sw=4: */
