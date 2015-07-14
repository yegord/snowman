/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "ArmArchitecture.h"

#include <nc/common/make_unique.h>

#include <nc/core/MasterAnalyzer.h>

#include "ArmDisassembler.h"
#include "ArmInstruction.h"
#include "ArmInstructionAnalyzer.h"
#include "ArmRegisters.h"
#include "CallingConventions.h"

namespace nc {
namespace arch {
namespace arm {

ArmArchitecture::ArmArchitecture(ByteOrder byteOrder):
    byteOrder_(byteOrder)
{
    if (byteOrder == ByteOrder::LittleEndian) {
        setName(QLatin1String("arm-le"));
    } else {
        setName(QLatin1String("arm-be"));
    }
    setBitness(32);
    setMaxInstructionSize(ArmInstruction::maxSize());

    setRegisters(ArmRegisters::instance());

    static core::MasterAnalyzer masterAnalyzer;
    setMasterAnalyzer(&masterAnalyzer);

    addCallingConvention(std::make_unique<DefaultCallingConvention>());
}

ArmArchitecture::~ArmArchitecture() {}

ByteOrder ArmArchitecture::getByteOrder(core::ir::Domain domain) const {
    if (domain == core::ir::MemoryDomain::MEMORY ||
        domain == core::ir::MemoryDomain::STACK)
    {
        return byteOrder_;
    } else {
        return ByteOrder::LittleEndian;
    }
}

std::unique_ptr<core::arch::Disassembler> ArmArchitecture::createDisassembler() const {
    return std::make_unique<ArmDisassembler>(this);
}

std::unique_ptr<core::irgen::InstructionAnalyzer> ArmArchitecture::createInstructionAnalyzer() const {
    return std::make_unique<ArmInstructionAnalyzer>(this);
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
