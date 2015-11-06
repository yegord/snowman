/* The file is part of Snowman decompiler. */
/* Copyright Markus Gothe <nietzsche@lysator.liu.se> 2015 */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/irgen/InstructionAnalyzer.h>

namespace nc {
namespace arch {
namespace mips {

class MipsArchitecture;
class MipsInstructionAnalyzerImpl;

class MipsInstructionAnalyzer: public core::irgen::InstructionAnalyzer {
    std::unique_ptr<MipsInstructionAnalyzerImpl> impl_;

public:
    MipsInstructionAnalyzer(const MipsArchitecture *architecture);

    ~MipsInstructionAnalyzer();

protected:
    virtual void doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) override;
};

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */
