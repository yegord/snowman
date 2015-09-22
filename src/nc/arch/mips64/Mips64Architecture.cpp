/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Mips64Architecture.h"

#include <nc/common/make_unique.h>

#include <nc/core/MasterAnalyzer.h>

#include "Mips64Disassembler.h"
#include "Mips64Instruction.h"
#include "Mips64InstructionAnalyzer.h"
#include "Mips64Registers.h"
#include "CallingConventions.h"

namespace nc {
namespace arch {
namespace mips64 {

Mips64Architecture::Mips64Architecture(ByteOrder byteOrder):
    byteOrder_(byteOrder)
{
    if (byteOrder == ByteOrder::LittleEndian) {
        setName(QLatin1String("mips64-le"));
    } else {
        setName(QLatin1String("mips64-be"));
    }
    setBitness(64);
    setMaxInstructionSize(Mips64Instruction::maxSize());

    setRegisters(Mips64Registers::instance());

    static core::MasterAnalyzer masterAnalyzer;
    setMasterAnalyzer(&masterAnalyzer);

    addCallingConvention(std::make_unique<n32n64CallingConvention>(this));
}

Mips64Architecture::~Mips64Architecture() {}

ByteOrder Mips64Architecture::getByteOrder(core::ir::Domain domain) const {
        return byteOrder_;
}

bool Mips64Architecture::isGlobalMemory(const core::ir::MemoryLocation &memoryLocation) const {
	/* Check if the register is non-volatile like $gp. */
	bool nonVolatileRegister = false;
	foreach (const auto &nonVolatileMemoryLocation, this->getCallingConvention(QLatin1String("n32n64"))->nonVolatileRegisterLocations()) {
      	if(memoryLocation == nonVolatileMemoryLocation){
      		nonVolatileRegister = true;
      		break;
      	}
	}
    
    return ((memoryLocation.domain() == core::ir::MemoryDomain::MEMORY) || nonVolatileRegister);
}

std::unique_ptr<core::arch::Disassembler> Mips64Architecture::createDisassembler() const {
    return std::make_unique<Mips64Disassembler>(this);
}

std::unique_ptr<core::irgen::InstructionAnalyzer> Mips64Architecture::createInstructionAnalyzer() const {
    return std::make_unique<Mips64InstructionAnalyzer>(this);
}

}}} // namespace nc::arch::mips64

/* vim:set et sts=4 sw=4: */
