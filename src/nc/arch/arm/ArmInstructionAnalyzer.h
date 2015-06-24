/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/irgen/InstructionAnalyzer.h>

namespace nc {
namespace arch {
namespace arm {

class ArmArchitecture;
class ArmInstructionAnalyzerImpl;

class ArmInstructionAnalyzer: public core::irgen::InstructionAnalyzer {
    std::unique_ptr<ArmInstructionAnalyzerImpl> impl_;

public:
    ArmInstructionAnalyzer(const ArmArchitecture *architecture);

    ~ArmInstructionAnalyzer();

protected:
    virtual void doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) override;
};

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
