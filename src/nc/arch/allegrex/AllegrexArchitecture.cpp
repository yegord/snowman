/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "AllegrexArchitecture.h"

#include <nc/common/make_unique.h>

#include <nc/core/MasterAnalyzer.h>

#include "AllegrexDisassembler.h"
#include "AllegrexInstruction.h"
#include "AllegrexInstructionAnalyzer.h"
#include "AllegrexRegisters.h"
#include "AllegrexCallingConventions.h"

namespace nc {
namespace arch {
namespace allegrex {

AllegrexArchitecture::AllegrexArchitecture(ByteOrder byteOrder):
    byteOrder_(byteOrder)
{
    assert(byteOrder == ByteOrder::LittleEndian);

    setName(QLatin1String("allegrex"));
    setBitness(32);
    setMaxInstructionSize(AllegrexInstruction::maxSize());

    setRegisters(AllegrexRegisters::instance());

    static core::MasterAnalyzer masterAnalyzer;
    setMasterAnalyzer(&masterAnalyzer);

    addCallingConvention(std::make_unique<DefaultCallingConvention>(this));
}

AllegrexArchitecture::~AllegrexArchitecture() {}

ByteOrder AllegrexArchitecture::getByteOrder(core::ir::Domain domain) const {
    return ByteOrder::LittleEndian;
}

std::unique_ptr<core::arch::Disassembler> AllegrexArchitecture::createDisassembler() const {
    return std::make_unique<AllegrexDisassembler>(this);
}

std::unique_ptr<core::irgen::InstructionAnalyzer> AllegrexArchitecture::createInstructionAnalyzer() const {
    return std::make_unique<AllegrexInstructionAnalyzer>(this);
}

}}} // namespace nc::arch::allegrex

/* vim:set et sts=4 sw=4: */
