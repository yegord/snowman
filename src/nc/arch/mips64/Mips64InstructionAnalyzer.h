/* The file is part of Snowman decompiler. */
/* Copyright Markus Gothe <nietzsche@lysator.liu.se> 2015 */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/irgen/InstructionAnalyzer.h>

namespace nc {
namespace arch {
namespace mips64 {

class Mips64Architecture;
class Mips64InstructionAnalyzerImpl;

class Mips64InstructionAnalyzer: public core::irgen::InstructionAnalyzer {
    std::unique_ptr<Mips64InstructionAnalyzerImpl> impl_;

public:
    Mips64InstructionAnalyzer(const Mips64Architecture *architecture);

    ~Mips64InstructionAnalyzer();

protected:
    virtual void doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) override;
};

}}} // namespace nc::arch::mips64

/* vim:set et sts=4 sw=4: */
