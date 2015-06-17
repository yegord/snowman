/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/arch/Architecture.h>

namespace nc {
namespace arch {
namespace mips {

/**
 * 32-bit MIPS Architecture.
 */
class MipsArchitecture: public nc::core::arch::Architecture {
    ByteOrder byteOrder_;
    std::unique_ptr<core::MasterAnalyzer> masterAnalyzer_;

public:
    /**
     * Constructor.
     *
     * \param byteOrder Byte order of the main memory.
     */
    explicit
    MipsArchitecture(ByteOrder byteOrder);

    virtual ~MipsArchitecture();

    /**
     * \return Byte order of the main memory.
     */
    ByteOrder byteOrder() const { return byteOrder_; }

    ByteOrder getByteOrder(core::ir::Domain domain) const override;
    std::unique_ptr<core::arch::Disassembler> createDisassembler() const override;
    std::unique_ptr<core::irgen::InstructionAnalyzer> createInstructionAnalyzer() const override;
};

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */
