/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "ArchitectureRepository.h"

#include <cassert>

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/arch/x86/X86Architecture.h>
#include <nc/arch/arm/ArmArchitecture.h>

#include "Architecture.h"

namespace nc { namespace core { namespace arch {

namespace {

ArchitectureRepository *createInstance() {
    using nc::arch::arm::ArmArchitecture;
    using nc::arch::x86::X86Architecture;

    static ArchitectureRepository result;
    result.registerArchitecture(std::make_unique<ArmArchitecture>(ByteOrder::LittleEndian));
    result.registerArchitecture(std::make_unique<ArmArchitecture>(ByteOrder::BigEndian));
    result.registerArchitecture(std::make_unique<X86Architecture>(X86Architecture::REAL_MODE));
    result.registerArchitecture(std::make_unique<X86Architecture>(X86Architecture::PROTECTED_MODE));
    result.registerArchitecture(std::make_unique<X86Architecture>(X86Architecture::LONG_MODE));
    return &result;
}

} // anonymous namespace

ArchitectureRepository *ArchitectureRepository::instance() {
    static auto repository = createInstance();
    return repository;
}

void ArchitectureRepository::registerArchitecture(std::unique_ptr<Architecture> architecture) {
    assert(architecture != nullptr);
    assert(!getArchitecture(architecture->name()) && "Cannot register two architectures with the same name.");

    architectures_.push_back(std::move(architecture));
}

const Architecture *ArchitectureRepository::getArchitecture(const QString &name) const {
    foreach (auto architecture, architectures()) {
        if (architecture->name() == name) {
            return architecture;
        }
    }
    return nullptr;
}

const std::vector<const Architecture *> &ArchitectureRepository::architectures() const {
    return reinterpret_cast<const std::vector<const Architecture *> &>(architectures_);
}

}}} // namespace nc::core::arch

/* vim:set et sts=4 sw=4: */
