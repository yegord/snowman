/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "MipsArchitecture.h"

#include <nc/common/make_unique.h>

#include <nc/core/MasterAnalyzer.h>

#include "MipsDisassembler.h"
#include "MipsInstruction.h"
#include "MipsInstructionAnalyzer.h"
#include "MipsRegisters.h"
#include "CallingConventions.h"

namespace nc {
namespace arch {
namespace mips {

MipsArchitecture::MipsArchitecture(ByteOrder byteOrder):
    byteOrder_(byteOrder)
{
    if (byteOrder == ByteOrder::LittleEndian) {
        setName(QLatin1String("mips-le"));
    } else {
        setName(QLatin1String("mips-be"));
    }
    setBitness(32);
    setMaxInstructionSize(MipsInstruction::maxSize());

    setRegisters(MipsRegisters::instance());

    static core::MasterAnalyzer masterAnalyzer;
    setMasterAnalyzer(&masterAnalyzer);

    addCallingConvention(std::make_unique<o32CallingConvention>(this));
}

MipsArchitecture::~MipsArchitecture() {}

ByteOrder MipsArchitecture::getByteOrder(core::ir::Domain domain) const {
        return byteOrder_;
}

bool MipsArchitecture::isGlobalMemory(const core::ir::MemoryLocation &memoryLocation) const {
	/* Check if the register is non-volatile like $gp. */
	bool nonVolatileRegister = false;
	foreach (const auto &nonVolatileMemoryLocation, this->getCallingConvention(QLatin1String("o32"))->nonVolatileRegisterLocations()) {
      	if(memoryLocation == nonVolatileMemoryLocation){
      		nonVolatileRegister = true;
      		break;
      	}
	}
    
    return ((memoryLocation.domain() == core::ir::MemoryDomain::MEMORY) || nonVolatileRegister);
}

std::unique_ptr<core::arch::Disassembler> MipsArchitecture::createDisassembler() const {
    return std::make_unique<MipsDisassembler>(this);
}

std::unique_ptr<core::irgen::InstructionAnalyzer> MipsArchitecture::createInstructionAnalyzer() const {
    return std::make_unique<MipsInstructionAnalyzer>(this);
}

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */
