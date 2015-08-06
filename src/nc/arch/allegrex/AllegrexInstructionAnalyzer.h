/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/irgen/InstructionAnalyzer.h>

namespace nc {
    namespace arch {
        namespace allegrex {

            class AllegrexArchitecture;
            class AllegrexInstructionAnalyzerImpl;

            class AllegrexInstructionAnalyzer : public core::irgen::InstructionAnalyzer {
                std::unique_ptr<AllegrexInstructionAnalyzerImpl> impl_;

            public:
                AllegrexInstructionAnalyzer(const AllegrexArchitecture *architecture);

                ~AllegrexInstructionAnalyzer();

            protected:
                virtual void doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) override;
            };

        }
    }
} // namespace nc::arch::allegrex

/* vim:set et sts=4 sw=4: */
