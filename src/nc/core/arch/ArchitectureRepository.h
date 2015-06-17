/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>
#include <vector>

#include <QString>

namespace nc { namespace core { namespace arch {

class Architecture;

/**
 * Static repository of architectures.
 */
class ArchitectureRepository {
    std::vector<std::unique_ptr<Architecture>> architectures_;

public:
    /**
     * \returns Architecture repository instance.
     */
    static ArchitectureRepository *instance();

    /**
     * Registers an architecture. The architecture must have a name different
     * from the already registered architectures.
     *
     * \param[in] architecture Valid pointer to an architecture.
     */
    void registerArchitecture(std::unique_ptr<Architecture> architecture);

    /**
     * \param[in] name Name of the architecture.
     *
     * \returns Valid pointer to the architecture with the given name,
     *          or nullptr if no such architecture found.
     */
    const Architecture *getArchitecture(const QString &name) const;

    /**
     * \returns List of all registered architectures.
     */
    const std::vector<const Architecture *> &architectures() const;
};

}}} // namespace nc::core::arch

/* vim:set et sts=4 sw=4: */
