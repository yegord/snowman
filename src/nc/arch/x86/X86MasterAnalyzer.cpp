/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "X86MasterAnalyzer.h"

#include <nc/common/Foreach.h>
#include <nc/common/StringToInt.h>
#include <nc/common/make_unique.h>

#include <nc/core/Context.h>
#include <nc/core/arch/Capstone.h>
#include <nc/core/image/Image.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calling/Conventions.h>
#include <nc/core/ir/calling/Hooks.h>
#include <nc/core/ir/dflow/Dataflows.h>

#include "X86Architecture.h"
#include "X86Instruction.h"
#include "X86Registers.h"

#include "udis86.h"

namespace nc {
namespace arch {
namespace x86 {

void X86MasterAnalyzer::createProgram(core::Context &context) const {
    MasterAnalyzer::createProgram(context);

    /*
     * Patch the IR to implement x86-64 implicit zero extend.
     */
    if (context.image()->architecture()->bitness() == 64) {
        auto minDomain = X86Registers::rax()->memoryLocation().domain();
        auto maxDomain = X86Registers::r15()->memoryLocation().domain();

        auto program = const_cast<core::ir::Program *>(context.program());

        foreach (auto *basicBlock, program->basicBlocks()) {
            context.cancellationToken().poll();

            foreach (auto statement, basicBlock->statements()) {
                if (auto assignment = statement->asAssignment()) {
                    if (auto access = assignment->left()->asMemoryLocationAccess()) {
                        if (minDomain <= access->memoryLocation().domain() &&
                            access->memoryLocation().domain() <= maxDomain &&
                            access->memoryLocation().addr() == 0 &&
                            access->memoryLocation().size() == 32)
                        {
                            auto patch = std::make_unique<core::ir::Assignment>(
                                    std::make_unique<core::ir::MemoryLocationAccess>(access->memoryLocation().shifted(32)),
                                    std::make_unique<core::ir::Constant>(SizedValue(32, 0)));
                            patch->setInstruction(statement->instruction());

                            basicBlock->insertAfter(statement, std::move(patch));
                        }
                    }
                }
            }
        }
    }
}

void X86MasterAnalyzer::detectCallingConventions(core::Context &context) const {
    context.logToken().info(tr("Detecting calling conventions."));

    auto architecture = context.image()->architecture();

    if (architecture->bitness() == 32) {
        using core::ir::calling::CalleeId;
        using core::ir::calling::EntryAddress;

        auto stdcall32 = architecture->getCallingConvention(QLatin1String("stdcall32"));

        foreach (auto symbol, context.image()->symbols()) {
            if (!symbol->value()) {
                continue;
            }
            auto index = symbol->name().lastIndexOf(QChar('@'));
            if (index == -1) {
                continue;
            }
            auto argumentsSize = stringToInt<ByteSize>(symbol->name().mid(index + 1));
            if (!argumentsSize) {
                continue;
            }
            CalleeId calleeId(EntryAddress(*symbol->value()));
            context.conventions()->setConvention(calleeId, stdcall32);
            context.conventions()->setStackArgumentsSize(calleeId, *argumentsSize);
        }

        ud_t ud_obj_;
        ud_init(&ud_obj_);
        ud_set_mode(&ud_obj_, context.image()->architecture()->bitness());

        foreach (auto function, context.functions()->list()) {
            if (!function->entry()->address()) {
                continue;
            }
            foreach (auto basicBlock, function->basicBlocks()) {
                auto terminator = basicBlock->getTerminator();
                if (!terminator) {
                    continue;
                }
                auto instruction = checked_cast<const X86Instruction *>(terminator->instruction());
                if (!instruction) {
                    continue;
                }

                ud_set_pc(&ud_obj_, instruction->addr());
                ud_set_input_buffer(&ud_obj_, const_cast<uint8_t *>(instruction->bytes()),
                                    checked_cast<std::size_t>(instruction->size()));
                ud_disassemble(&ud_obj_);

                assert(ud_obj_.mnemonic != UD_Iinvalid);

                if (ud_obj_.mnemonic != UD_Iret) {
                    continue;
                }

                if (ud_obj_.operand[0].type == UD_NONE) {
                    continue;
                }
                assert(ud_obj_.operand[0].type == UD_OP_IMM && ud_obj_.operand[0].size == 16);

                CalleeId calleeId(EntryAddress(*function->entry()->address()));
                context.conventions()->setConvention(calleeId, stdcall32);
                context.conventions()->setStackArgumentsSize(calleeId, ud_obj_.operand[0].lval.uword);
            }
        }
    }
}

void X86MasterAnalyzer::detectCallingConvention(core::Context &context, const core::ir::calling::CalleeId &calleeId) const {
    auto architecture = context.image()->architecture();

    auto setConvention = [&](const char *name) {
        context.conventions()->setConvention(calleeId, architecture->getCallingConvention(QLatin1String(name)));
    };

    switch (architecture->bitness()) {
        case 16:
            setConvention("cdecl16");
            break;
        case 32:
            setConvention("cdecl32");
            break;
        case 64:
            setConvention("amd64");
            break;
    }
}

} // namespace x86
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
