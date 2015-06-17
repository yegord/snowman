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

#include "X86InstructionAnalyzer.h"

#include <nc/common/CheckedCast.h>
#include <nc/common/Unreachable.h>

#include <nc/core/arch/Capstone.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/irgen/Expressions.h>
#include <nc/core/irgen/InvalidInstructionException.h>

#include "X86Architecture.h"
#include "X86Instruction.h"
#include "X86Registers.h"

namespace nc {
namespace arch {
namespace x86 {

namespace {

class X86ExpressionFactory: public core::irgen::expressions::ExpressionFactory<X86ExpressionFactory> {
public:
    X86ExpressionFactory(const core::arch::Architecture *architecture):
        core::irgen::expressions::ExpressionFactory<X86ExpressionFactory>(architecture)
    {}
};

typedef core::irgen::expressions::ExpressionFactoryCallback<X86ExpressionFactory> X86ExpressionFactoryCallback;

NC_DEFINE_REGISTER_EXPRESSION(X86Registers, cf)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, pf)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, zf)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, sf)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, of)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, af)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, df)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, flags)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, eflags)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, rflags)

NC_DEFINE_REGISTER_EXPRESSION(X86Registers, pseudo_flags)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, less)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, less_or_equal)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, below_or_equal)

NC_DEFINE_REGISTER_EXPRESSION(X86Registers, cx)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, ecx)
NC_DEFINE_REGISTER_EXPRESSION(X86Registers, rcx)

core::irgen::expressions::MemoryLocationExpression
resizedRegister(const core::arch::Register *reg, SmallBitSize size) {
    return core::ir::MemoryLocation(reg->memoryLocation().domain(), 0, size);
}

core::irgen::expressions::MemoryLocationExpression
temporary(SmallBitSize size) {
    return resizedRegister(X86Registers::tmp64(), size);
}

} // anonymous namespace

class X86InstructionAnalyzerImpl {
    Q_DECLARE_TR_FUNCTIONS(X86InstructionAnalyzerImpl)

    const X86Architecture *architecture_;
    const X86Instruction *instruction_;
    core::arch::Capstone capstone_;
    core::arch::CapstoneInstructionPtr instr_;
    const cs_x86 *detail_;

public:
    explicit
    X86InstructionAnalyzerImpl(const X86Architecture *architecture):
        architecture_(architecture), capstone_(CS_ARCH_X86, 0)
    {
        assert(architecture != nullptr);
    }

    void createStatements(const X86Instruction *instruction, core::ir::Program *program) {
        assert(instruction != nullptr);
        assert(program != nullptr);

        instruction_ = instruction;

        instr_ = disassemble(instruction);
        assert(instr_ != nullptr);
        detail_ = &instr_->detail->x86;

        core::ir::BasicBlock *cachedDirectSuccessor = nullptr;
        auto directSuccessor = [&]() -> core::ir::BasicBlock * {
            if (!cachedDirectSuccessor) {
                cachedDirectSuccessor = program->createBasicBlock(instruction->endAddr());
            }
            return cachedDirectSuccessor;
        };

        X86ExpressionFactory factory(architecture_);
        X86ExpressionFactoryCallback _(factory, program->getBasicBlockForInstruction(instruction), instruction);

        using namespace core::irgen::expressions;

        /* Describing semantics */
        switch (instr_->id) {
            case X86_INS_ADC: {
                _[
                    operand(0) ^= operand(0) + operand(1) + zero_extend(cf),
                    cf ^= intrinsic(),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= signed_(operand(0)) < constant(0),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    kill(pseudo_flags)
                ];
                break;
            }
            case X86_INS_ADD: {
                _[
                    operand(0) ^= operand(0) + operand(1),
                    cf ^= intrinsic(),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= signed_(operand(0)) < constant(0),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    kill(pseudo_flags)
                ];
                break;
            }
            case X86_INS_AND: {
                if (operandsAreTheSame(0, 1)) {
                    _[operand(0) ^= operand(0)];
                } else {
                    _[operand(0) ^= operand(0) & operand(1)];
                }

                _[
                    cf ^= constant(0),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= intrinsic(),
                    of ^= constant(0),
                    af ^= undefined(),
                    kill(pseudo_flags)
                ];
                break;
            }
            case X86_INS_BOUND: {
                /* Deprecated, used mostly for debugging, it's better to generate no IR code at all. */
                break;
            }
            case X86_INS_BT: {
                _[
                    cf ^= truncate(unsigned_(operand(0)) >> operand(1)),
                    of ^= undefined(),
                    sf ^= undefined(),
                    zf ^= undefined(),
                    af ^= undefined(),
                    pf ^= undefined(),
                    kill(pseudo_flags)
                ];
                break;
            }
            case X86_INS_CALL: {
                auto sp = architecture_->stackPointer();
                auto ip = architecture_->instructionPointer();

                _[
                    regizter(sp) ^= regizter(sp) - constant(ip->size() / CHAR_BIT),
                    *regizter(sp) ^= constant(instruction->endAddr(), ip->size()),
                    call(operand(0)),
                    regizter(sp) ^= regizter(sp) + constant(ip->size() / CHAR_BIT)
                ];
                break;
            }
            case X86_INS_CBW: {
                _[
                    regizter(X86Registers::ax()) ^= sign_extend(regizter(X86Registers::al()))
                ];
                break;
            }
            case X86_INS_CWDE: {
                _[
                    regizter(X86Registers::eax()) ^= sign_extend(regizter(X86Registers::ax()))
                ];
                break;
            }
            case X86_INS_CDQE: {
                _[
                    regizter(X86Registers::rax()) ^= sign_extend(regizter(X86Registers::eax()))
                ];
                break;
            }
            case X86_INS_CLD: {
                _[
                    df ^= constant(0)
                ];
                break;
            }
            case X86_INS_CMP: {
                _[
                    cf ^= unsigned_(operand(0)) < operand(1),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == operand(1),
                    sf ^= signed_(operand(0)) < operand(1),
                    of ^= intrinsic(),
                    af ^= intrinsic(),

                    less             ^= signed_(operand(0)) < operand(1),
                    less_or_equal    ^= signed_(operand(0)) <= operand(1),
                    below_or_equal   ^= unsigned_(operand(0)) <= operand(1)
                ];
                break;
            }
            case X86_INS_CMPSB: case X86_INS_CMPSW: case X86_INS_CMPSD: case X86_INS_CMPSQ:
            case X86_INS_MOVSB: case X86_INS_MOVSW: case X86_INS_MOVSD: case X86_INS_MOVSQ:
            case X86_INS_SCASB: case X86_INS_SCASW: case X86_INS_SCASD: case X86_INS_SCASQ:
            case X86_INS_STOSB: case X86_INS_STOSW: case X86_INS_STOSD: case X86_INS_STOSQ: {
                SmallBitSize accessSize;

                switch (instr_->id) {
                    case X86_INS_CMPSB: case X86_INS_MOVSB: case X86_INS_SCASB: case X86_INS_STOSB:
                        accessSize = 8;
                        break;
                    case X86_INS_CMPSW: case X86_INS_MOVSW: case X86_INS_SCASW: case X86_INS_STOSW:
                        accessSize = 16;
                        break;
                    case X86_INS_CMPSD: case X86_INS_MOVSD: case X86_INS_SCASD: case X86_INS_STOSD:
                        accessSize = 32;
                        break;
                    case X86_INS_CMPSQ: case X86_INS_MOVSQ: case X86_INS_SCASQ: case X86_INS_STOSQ:
                        accessSize = 64;
                        break;
                    default:
                        unreachable();
                }

                auto di = resizedRegister(X86Registers::di(), addressSize());
                auto si = resizedRegister(X86Registers::si(), addressSize());
                auto cx = resizedRegister(X86Registers::cx(), operandSize());

                auto increment = temporary(addressSize());
                _[
                    increment ^= constant(accessSize / CHAR_BIT) - constant(2 * accessSize / CHAR_BIT, si.memoryLocation().size()) * zero_extend(df)
                ];

                X86ExpressionFactoryCallback condition(factory, program->createBasicBlock(), instruction);
                X86ExpressionFactoryCallback body(factory, program->createBasicBlock(), instruction);

                _[jump(condition.basicBlock())];

                /* If we have a REP* prefix. */
                if (detail_->prefix[0]) {
                    condition[jump(cx, body.basicBlock(), directSuccessor())];

                    body[cx ^= cx - constant(1)];
                } else {
                    condition[jump(body.basicBlock())];
                }

                bool conditional = false;
                switch (instr_->id) {
                    case X86_INS_CMPSB: case X86_INS_CMPSW: case X86_INS_CMPSD: case X86_INS_CMPSQ: {
                        auto left = dereference(si, accessSize);
                        auto right = dereference(di, accessSize);

                        body[
                            cf ^= unsigned_(left) < right,
                            pf ^= intrinsic(),
                            zf ^= left == right,
                            sf ^= signed_(left) < right,
                            of ^= intrinsic(),
                            af ^= intrinsic(),

                            less             ^= signed_(left) < right,
                            less_or_equal    ^= signed_(left) <= right,
                            below_or_equal   ^= unsigned_(left) <= right
                        ];

                        conditional = true;
                        break;
                    }
                    case X86_INS_MOVSB: case X86_INS_MOVSW: case X86_INS_MOVSD: case X86_INS_MOVSQ: {
                        body[dereference(di, accessSize) ^= *si];
                        break;
                    }
                    case X86_INS_SCASB: case X86_INS_SCASW: case X86_INS_SCASD: case X86_INS_SCASQ: {
                        auto left = dereference(di, accessSize);
                        auto right = resizedRegister(X86Registers::ax(), accessSize);

                        body[
                            cf ^= unsigned_(left) < right,
                            pf ^= intrinsic(),
                            zf ^= left == right,
                            sf ^= signed_(left) < right,
                            of ^= intrinsic(),
                            af ^= intrinsic(),

                            less             ^= signed_(left) < right,
                            less_or_equal    ^= signed_(left) <= right,
                            below_or_equal   ^= unsigned_(left) <= right
                        ];
                        conditional = true;
                        break;
                    }
                    case X86_INS_STOSB: case X86_INS_STOSW: case X86_INS_STOSD: case X86_INS_STOSQ: {
                        body[dereference(di, accessSize) ^= resizedRegister(X86Registers::ax(), accessSize)];
                        break;
                    }
                    default:
                        unreachable();
                }

                body[
                    di ^= di + increment,
                    si ^= si + increment
                ];

                if (detail_->prefix[0] == X86_PREFIX_REP) {
                    if (conditional) {
                        /* REPE */
                        body[jump(zf, condition.basicBlock(), directSuccessor())];
                    } else {
                        /* REP */
                        body[jump(condition.basicBlock())];
                    }
                } else if (detail_->prefix[0] == X86_PREFIX_REPNE) {
                    /* REPNE */
                    body[jump(~zf, condition.basicBlock(), directSuccessor())];
                } else {
                    body[jump(directSuccessor())];
                }
                break;
            }
            case X86_INS_CMPXCHG: {
                X86ExpressionFactoryCallback then(factory, program->createBasicBlock(), instruction);

                _[
                    cf ^= intrinsic(),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == operand(1),
                    sf ^= intrinsic(),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    kill(pseudo_flags),

                    jump(zf, then.basicBlock(), directSuccessor())
                ];

                auto operand0 = operand(0);
                auto tmp = temporary(operand0.size());
                then[
                    tmp ^= std::move(operand0),
                    operand(0) ^= operand(1),
                    operand(1) ^= tmp,
                    jump(directSuccessor())
                ];

                break;
            }
            case X86_INS_CPUID: {
                _[
                    regizter(X86Registers::eax()) ^= intrinsic(),
                    regizter(X86Registers::ebx()) ^= intrinsic(),
                    regizter(X86Registers::ecx()) ^= intrinsic(),
                    regizter(X86Registers::edx()) ^= intrinsic()
                ];
                break;
            }
            case X86_INS_DEC: {
                _[
                    operand(0) ^= operand(0) - constant(1),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= signed_(operand(0)) < constant(0),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    kill(pseudo_flags)
                ];
                break;
            }
            case X86_INS_DIV: {
                auto operand0 = operand(0);
                auto size = std::max(operand0.size(), 16);
                auto ax = resizedRegister(X86Registers::ax(), size);
                auto dx = resizedRegister(X86Registers::dx(), size);

                if (operand0.size() >= 16) {
                    _[
                        dx ^= unsigned_(ax) % std::move(operand0),
                        ax ^= unsigned_(ax) / operand(0)
                    ];
                } else {
                    _[
                        dx ^= unsigned_(ax) % sign_extend(std::move(operand0)),
                        ax ^= unsigned_(ax) / sign_extend(operand(0))
                    ];
                }
                break;
            }
            case X86_INS_HLT: {
                _(std::make_unique<core::ir::InlineAssembly>());
                _[halt()];
                break;
            }
            case X86_INS_IDIV: {
    #if 0
                /* A correct implementation, however, generating complicated code. */

                auto size = std::max(instr->operand(0)->size(), 16);
                auto ax = resizedRegister(X86Registers::ax(), size);
                auto dx = resizedRegister(X86Registers::dx(), size);
                auto tmp = temporary(size * 2);

                _[
                    tmp ^= (zero_extend(dx, size * 2) << constant(size)) | zero_extend(ax, size * 2),
                    dx ^= truncate(signed_(tmp) % sign_extend(operand(0))),
                    ax ^= truncate(signed_(tmp) / sign_extend(operand(0)))
                ];
    #endif

                auto operand0 = operand(0);
                auto size = std::max(operand0.size(), 16);
                auto ax = resizedRegister(X86Registers::ax(), size);
                auto dx = resizedRegister(X86Registers::dx(), size);

                if (operand0.size() >= 16) {
                    _[
                        dx ^= signed_(ax) % std::move(operand0),
                        ax ^= signed_(ax) / operand(0)
                    ];
                } else {
                    _[
                        dx ^= signed_(ax) % sign_extend(std::move(operand0)),
                        ax ^= signed_(ax) / sign_extend(operand(0))
                    ];
                }

                _[
                    cf ^= undefined(),
                    of ^= undefined(),
                    sf ^= undefined(),
                    zf ^= undefined(),
                    af ^= undefined(),
                    pf ^= undefined()
                ];
                break;
            }
            case X86_INS_INT: {
                auto term = createTermForOperand(0);
                if (auto constant = term->asConstant()) {
                    if (constant->value().value() == 3) {
                        /* int 3 is debug break, remove it. */
                        break;
                    }
                }
                _(std::make_unique<core::ir::InlineAssembly>());
                break;
            }
            case X86_INS_INT3: {
                /* int 3 is a debug break, remove it. */
                break;
            }
            case X86_INS_IMUL: case X86_INS_MUL: {
                /* All compilers always use IMUL, not MUL. Even for unsigned operands.
                 * See http://stackoverflow.com/questions/4039378/x86-mul-instruction-from-vs-2008-2010
                 *
                 * So, there is no such thing as signed or unsigned multiplication.
                 */
                if (detail_->op_count == 1) {
                    /* result2:result1 = arg0 * op0 */
                    const core::arch::Register *arg0;
                    const core::arch::Register *result1;
                    const core::arch::Register *result2;

                    auto operand0 = operand(0);
                    switch (operand0.size()) {
                        case 8:
                            arg0 = X86Registers::al();
                            result1 = X86Registers::ax();
                            result2 = 0;
                            break;
                        case 16:
                            arg0 = X86Registers::ax();
                            result1 = X86Registers::ax();
                            result2 = X86Registers::dx();
                            break;
                        case 32:
                            arg0 = X86Registers::eax();
                            result1 = X86Registers::eax();
                            result2 = X86Registers::edx();
                            break;
                        case 64:
                            arg0 = X86Registers::rax();
                            result1 = X86Registers::rax();
                            result2 = X86Registers::rdx();
                            break;
                        default:
                            throw core::irgen::InvalidInstructionException(tr("Strange argument size"));
                    }

                    if (result1->size() == arg0->size()) {
                        _[regizter(result1) ^= regizter(arg0) * std::move(operand0)];
                    } else {
                        _[regizter(result1) ^= sign_extend(regizter(arg0)) * sign_extend(std::move(operand0))];
                    }
                    if (result2) {
                        _[regizter(result2) ^= intrinsic()];
                    }
                } else if (detail_->op_count == 2) {
                    _[operand(0) ^= operand(0) * operand(1)];
                } else {
                    /* Three operands. */
                    _[operand(0) ^= operand(1) * operand(2)];
                }

                _[
                    cf ^= intrinsic(),
                    of ^= intrinsic(),
                    sf ^= undefined(),
                    zf ^= undefined(),
                    af ^= undefined(),
                    pf ^= undefined()
                ];
                break;
            }
            case X86_INS_INC: {
                _[
                    operand(0) ^= operand(0) + constant(1),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= signed_(operand(0)) < constant(0),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    kill(pseudo_flags)
                ];
                break;
            }
            case X86_INS_JA: {
                _[jump(~choice(below_or_equal, cf | zf), operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JAE: {
                _[jump(~cf, operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JB: {
                _[jump(cf, operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JBE: {
                _[jump(choice(below_or_equal, cf | zf), operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JCXZ: {
                _[jump(~(cx == constant(0)), operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JECXZ: {
                _[jump(~(ecx == constant(0)), operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JRCXZ: {
                _[jump(~(rcx == constant(0)), operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JE: {
                _[jump(zf, operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JG: {
                _[jump(~choice(less_or_equal, zf | ~(sf == of)), operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JGE: {
                _[jump(~choice(less, ~(sf == of)), operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JL: {
                _[jump(choice(less, ~(sf == of)), operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JLE: {
                _[jump(choice(less_or_equal, zf | ~(sf == of)), operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JNE: {
                _[jump(~zf, operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JNO: {
                _[jump(~of, operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JNP: {
                _[jump(~pf, operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JNS: {
                _[jump(~sf, operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JO: {
                _[jump(of, operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JP: {
                _[jump(pf, operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JS: {
                _[jump(sf, operand(0), directSuccessor())];
                break;
            }
            case X86_INS_JMP: {
                _[jump(operand(0))];
                break;
            }
            case X86_INS_CMOVA: case X86_INS_CMOVAE: case X86_INS_CMOVB: case X86_INS_CMOVBE:
            case X86_INS_CMOVE: case X86_INS_CMOVG: case X86_INS_CMOVGE: case X86_INS_CMOVL:
            case X86_INS_CMOVLE: case X86_INS_CMOVNE: case X86_INS_CMOVNO: case X86_INS_CMOVNP:
            case X86_INS_CMOVNS: case X86_INS_CMOVO: case X86_INS_CMOVP: case X86_INS_CMOVS: {
                X86ExpressionFactoryCallback then(factory, program->createBasicBlock(), instruction);

                switch (instr_->id) {
                    case X86_INS_CMOVA:
                        _[jump(~choice(below_or_equal, cf | zf), then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVAE:
                        _[jump(~cf, then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVB:
                        _[jump(cf, then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVBE:
                        _[jump(choice(below_or_equal, cf | zf), then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVE:
                        _[jump(zf, then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVG:
                        _[jump(~choice(less_or_equal, zf | ~(sf == of)), then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVGE:
                        _[jump(~choice(less, ~(sf == of)), then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVL:
                        _[jump(choice(less, ~(sf == of)), then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVLE:
                        _[jump(choice(less_or_equal, zf | ~(sf == of)), then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVNE:
                        _[jump(~zf, then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVNO:
                        _[jump(~of, then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVNP:
                        _[jump(~pf, then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVNS:
                        _[jump(~sf, then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVO:
                        _[jump(of, then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVP:
                        _[jump(pf, then.basicBlock(), directSuccessor())]; break;
                    case X86_INS_CMOVS:
                        _[jump(sf, then.basicBlock(), directSuccessor())]; break;
                    default: unreachable();
                }

                then[
                    operand(0) ^= operand(1),
                    jump(directSuccessor())
                ];

                break;
            }
            case X86_INS_LEA: {
                auto operand0 = operand(0);
                auto operand1 = core::irgen::expressions::TermExpression(createDereferenceAddress(detail_->operands[1]));

                if (operand0.size() == operand1.size()) {
                    _[std::move(operand0) ^= std::move(operand1)];
                } else if (operand0.size() < operand1.size()) {
                    _[std::move(operand0) ^= truncate(std::move(operand1))];
                } else if (operand0.size() > operand1.size()) {
                    _[std::move(operand0) ^= zero_extend(std::move(operand1))];
                }
                break;
            }
            case X86_INS_LEAVE: {
                auto sp = architecture_->stackPointer();
                auto bp = architecture_->basePointer();

                _[
                    regizter(sp) ^= regizter(bp),
                    regizter(bp) ^= *regizter(sp),
                    regizter(sp) ^= regizter(sp) + constant(bp->size() / CHAR_BIT)
                ];
                break;
            }
            case X86_INS_LOOP:
            case X86_INS_LOOPE:
            case X86_INS_LOOPNE: {
                auto cx = resizedRegister(X86Registers::cx(), addressSize());

                _[cx ^= cx - constant(1)];

                switch (instr_->id) {
                    case X86_INS_LOOP:
                        _[jump(~(cx == constant(0)), operand(0), directSuccessor())];
                        break;
                    case X86_INS_LOOPE:
                        _[jump(~(cx == constant(0)) & zf, operand(0), directSuccessor())];
                        break;
                    case X86_INS_LOOPNE:
                        _[jump(~(cx == constant(0)) & ~zf, operand(0), directSuccessor())];
                        break;
                    default:
                        unreachable();
                }
                break;
            }
            case X86_INS_MOV: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);

                if (operand0.size() == operand1.size()) {
                    _[std::move(operand0) ^= std::move(operand1)];
                } else if (operand0.size() > operand1.size()) {
                    /* For example, move of a small constant to a big register. */
                    _[std::move(operand0) ^= zero_extend(std::move(operand1))];
                } else {
                    /* Happens in assignments to segment registers. Known bug of udis86. */
                    _[std::move(operand0) ^= truncate(std::move(operand1))];
                }
                break;
            }
            case X86_INS_MOVSX: case X86_INS_MOVSXD: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);

                if (operand0.size() == operand1.size()) {
                    _[std::move(operand0) ^= std::move(operand1)];
                } else {
                    _[std::move(operand0) ^= sign_extend(std::move(operand1))];
                }
                break;
            }
            case X86_INS_MOVZX: {
                _[operand(0) ^= zero_extend(operand(1))];
                break;
            }
            case X86_INS_NEG: {
                _[
                    cf ^= ~(operand(0) == constant(0)),
                    operand(0) ^= -operand(0),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= intrinsic(),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    kill(pseudo_flags)
                ];
                break;
            }
            case X86_INS_NOP: {
                break;
            }
            case X86_INS_NOT: {
                _[operand(0) ^= ~operand(0)];
                break;
            }
            case X86_INS_OR: {
                if (operandsAreTheSame(0, 1)) {
                    _[operand(0) ^= operand(0)];
                } else {
                    _[operand(0) ^= operand(0) | operand(1)];
                }

                _[
                    cf ^= constant(0),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= intrinsic(),
                    of ^= constant(0),
                    af ^= undefined(),
                    kill(pseudo_flags)
                ];
                break;
            }
            case X86_INS_POP: {
                auto sp = architecture_->stackPointer();
                auto operand0 = operand(0);
                auto size = operand0.size();
                _[
                    std::move(operand0) ^= *regizter(sp),
                    regizter(sp) ^= regizter(sp) + constant(size / CHAR_BIT)
                ];
                break;
            }
            case X86_INS_PUSH: {
                auto sp = architecture_->stackPointer();
                const auto &op = detail_->operands[0];
                if (op.type == X86_OP_IMM) {
                    _[
                        regizter(sp) ^= regizter(sp) - constant(sp->size() / CHAR_BIT),
                        *regizter(sp) ^= constant(op.imm, sp->size())
                    ];
                } else {
                    auto operand0 = operand(0);
                    auto size = operand0.size();

                    _[
                        regizter(sp) ^= regizter(sp) - constant(size / CHAR_BIT),
                        *regizter(sp) ^= std::move(operand0)
                    ];
                }
                break;
            }
            case X86_INS_POPF: {
                auto sp = architecture_->stackPointer();
                _[
                    *regizter(sp) ^= flags,
                    regizter(sp) ^= regizter(sp) + constant(2)
                ];
                break;
            }
            case X86_INS_POPFD: {
                auto sp = architecture_->stackPointer();
                _[
                    eflags ^= *regizter(sp),
                    regizter(sp) ^= regizter(sp) + constant(4)
                ];
                break;
            }
            case X86_INS_POPFQ: {
                auto sp = architecture_->stackPointer();
                _[
                    rflags ^= *regizter(sp),
                    regizter(sp) ^= regizter(sp) + constant(8)
                ];
                break;
            }
            case X86_INS_PUSHF: {
                auto sp = architecture_->stackPointer();
                _[
                    regizter(sp) ^= regizter(sp) - constant(2),
                    *regizter(sp) ^= flags
                ];
                break;
            }
            case X86_INS_PUSHFD: {
                auto sp = architecture_->stackPointer();
                _[
                    regizter(sp) ^= regizter(sp) - constant(4),
                    *regizter(sp) ^= eflags & constant(0x00fcffff)
                ];
                break;
            }
            case X86_INS_PUSHFQ: {
                auto sp = architecture_->stackPointer();
                _[
                    regizter(sp) ^= regizter(sp) - constant(8),
                    *regizter(sp) ^= rflags & constant(0x00fcffff)
                ];
                break;
            }
            case X86_INS_RET: {
                auto sp = architecture_->stackPointer();
                auto ip = architecture_->instructionPointer();

                _[regizter(ip) ^= *regizter(sp)];
                /* sp is incremented in call instruction. */

                if (detail_->op_count == 1) {
                    auto operand0 = operand(0);
                    if (operand0.size() < sp->size()) {
                        _[regizter(sp) ^= regizter(sp) + zero_extend(std::move(operand0))];
                    } else {
                        _[regizter(sp) ^= regizter(sp) + std::move(operand0)];
                    }
                }

                _[jump(regizter(ip))];

                break;
            }
            case X86_INS_SHL: {
                if (detail_->op_count == 2) {
                    _[operand(0) ^= operand(0) << operand(1)];
                } else {
                    _[operand(0) ^= operand(0) << constant(1)];
                }

                _[
                    cf ^= intrinsic(),
                    sf ^= undefined(),
                    zf ^= operand(0) == constant(0),
                    pf ^= intrinsic()
                ];
                break;
            }
            case X86_INS_SAR: {
                if (detail_->op_count == 2) {
                    _[operand(0) ^= signed_(operand(0)) >> operand(1)];
                } else {
                    _[operand(0) ^= signed_(operand(0)) >> constant(1)];
                }

                _[
                    cf ^= intrinsic(),
                    sf ^= undefined(),
                    zf ^= operand(0) == constant(0),
                    pf ^= intrinsic()
                ];
                break;
            }
            case X86_INS_SBB: {
                _[
                    less           ^= signed_(operand(0))   <  operand(1) + zero_extend(cf),
                    less_or_equal  ^= signed_(operand(0))   <= operand(1) + zero_extend(cf),
                    cf             ^= unsigned_(operand(0)) <  operand(1) + zero_extend(cf),
                    below_or_equal ^= unsigned_(operand(0)) <= operand(1) + zero_extend(cf),

                    operand(0) ^= operand(0) - (operand(1) + zero_extend(cf)),

                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0) + zero_extend(cf),
                    sf ^= signed_(operand(0)) < constant(0) + zero_extend(cf),
                    of ^= intrinsic(),
                    af ^= intrinsic()
                ];
                break;
            }
            case X86_INS_SETA: {
                _[operand(0) ^= zero_extend(~choice(below_or_equal, cf | zf))];
                break;
            }
            case X86_INS_SETAE: {
                _[operand(0) ^= zero_extend(~cf)];
                break;
            }
            case X86_INS_SETB: {
                _[operand(0) ^= zero_extend(cf)];
                break;
            }
            case X86_INS_SETBE: {
                _[operand(0) ^= zero_extend(choice(below_or_equal, cf | zf))];
                break;
            }
            case X86_INS_SETE: {
                _[operand(0) ^= zero_extend(zf)];
                break;
            }
            case X86_INS_SETG: {
                _[operand(0) ^= zero_extend(~choice(less_or_equal, zf & ~(sf == of)))];
                break;
            }
            case X86_INS_SETGE: {
                _[operand(0) ^= zero_extend(~choice(less, ~(sf == of)))];
                break;
            }
            case X86_INS_SETL: {
                _[operand(0) ^= zero_extend(choice(less, ~(sf == of)))];
                break;
            }
            case X86_INS_SETLE: {
                _[operand(0) ^= zero_extend(choice(less_or_equal, zf | ~(sf == of)))];
                break;
            }
            case X86_INS_SETNE: {
                _[operand(0) ^= zero_extend(~zf)];
                break;
            }
            case X86_INS_SETNO: {
                _[operand(0) ^= zero_extend(~of)];
                break;
            }
            case X86_INS_SETNP: {
                _[operand(0) ^= zero_extend(~pf)];
                break;
            }
            case X86_INS_SETNS: {
                _[operand(0) ^= zero_extend(~sf)];
                break;
            }
            case X86_INS_SETO: {
                _[operand(0) ^= zero_extend(of)];
                break;
            }
            case X86_INS_SETP: {
                _[operand(0) ^= zero_extend(pf)];
                break;
            }
            case X86_INS_SETS: {
                _[operand(0) ^= zero_extend(sf)];
                break;
            }
            case X86_INS_SHR: {
                if (detail_->op_count == 2) {
                    _[operand(0) ^= unsigned_(operand(0)) >> operand(1)];
                } else {
                    _[operand(0) ^= unsigned_(operand(0)) >> constant(1)];
                }

                _[
                    cf ^= intrinsic(),
                    sf ^= undefined(),
                    zf ^= operand(0) == constant(0),
                    pf ^= intrinsic()
                ];
                break;
            }
            case X86_INS_STD: {
                _[
                    df ^= constant(1)
                ];
                break;
            }
            case X86_INS_SUB: {
                _[
                    less             ^= signed_(operand(0)) < operand(1),
                    less_or_equal    ^= signed_(operand(0)) <= operand(1),
                    cf               ^= unsigned_(operand(0)) < operand(1),
                    below_or_equal   ^= unsigned_(operand(0)) <= operand(1),

                    operand(0) ^= operand(0) - operand(1),

                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= signed_(operand(0)) < constant(0),
                    of ^= intrinsic(),
                    af ^= intrinsic()
                ];
                break;
            }
            case X86_INS_TEST: {
                _[
                    cf ^= constant(0),
                    pf ^= intrinsic()
                ];

                if (operandsAreTheSame(0, 1)) {
                    _[zf ^= operand(0) == constant(0)];
                    _[sf ^= signed_(operand(0)) < constant(0)];
                } else {
                    _[zf ^= (operand(0) & operand(1)) == constant(0)];
                    _[sf ^= signed_(operand(0) & operand(1)) < constant(0)];
                }

                _[
                    of ^= constant(0),
                    af ^= undefined(),
                    kill(pseudo_flags)
                ];
                break;
            }
            case X86_INS_XCHG: {
                auto operand0 = operand(0);
                auto tmp = temporary(operand0.size());

                _[
                    tmp ^= operand(0),
                    operand(0) ^= operand(1),
                    operand(1) ^= tmp
                ];
                break;
            }
            case X86_INS_XOR: {
                if (operandsAreTheSame(0, 1)) {
                    _[operand(0) ^= constant(0)];
                } else {
                    _[operand(0) ^= operand(0) ^ operand(1)];
                }

                _[
                    cf ^= constant(0),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= intrinsic(),
                    of ^= constant(0),
                    af ^= undefined(),
                    kill(pseudo_flags)
                ];
                break;
            }
            default: {
                /* Unsupported instruction. */
                _(std::make_unique<core::ir::InlineAssembly>());
                return;
            }
        }
    }

private:
    core::arch::CapstoneInstructionPtr disassemble(const X86Instruction *instruction) {
        capstone_.setMode(instruction->csMode());
        return capstone_.disassemble(instruction->addr(), instruction->bytes(), instruction->size());
    }

    core::irgen::expressions::TermExpression operand(std::size_t index) const {
        return core::irgen::expressions::TermExpression(createTermForOperand(index));
    }

    bool operandsAreTheSame(std::size_t index1, std::size_t index2) const {
        const auto &op1 = detail_->operands[index1];
        const auto &op2 = detail_->operands[index2];

        return op1.type == X86_OP_REG && op2.type == X86_OP_REG && op1.reg == op2.reg;
    }

    std::unique_ptr<core::ir::Term> createTermForOperand(std::size_t index) const {
        const auto &operand = detail_->operands[index];

        switch (operand.type) {
            case X86_OP_INVALID:
                throw core::irgen::InvalidInstructionException(tr("The instruction does not have an argument with index %1").arg(index));
            case X86_OP_REG: {
                auto result = createRegisterAccess(operand.reg);
                assert(result != nullptr);
                return result;
            }
            case X86_OP_IMM: {
                /* Signed number, sign-extended to match the size of the other operand. */
                return std::make_unique<core::ir::Constant>(SizedValue(operand.size * CHAR_BIT, operand.imm));
            }
            case X86_OP_MEM:
                return createDereference(operand);
            case X86_OP_FP:
                // TODO
                throw core::irgen::InvalidInstructionException(tr("We do not support floating-point constants yet."));
            default:
                unreachable();
        }
    }

    std::unique_ptr<core::ir::Term> createRegisterAccess(unsigned reg) const {
        switch (reg) {
        case X86_REG_INVALID: return nullptr;

        #define REG(cs_name, nc_name) case X86_REG_##cs_name: return X86InstructionAnalyzer::createTerm(X86Registers::nc_name());

        REG(AL, al)
        REG(CL, cl)
        REG(DL, dl)
        REG(BL, bl)
        REG(AH, ah)
        REG(CH, ch)
        REG(DH, dh)
        REG(BH, bh)
        REG(SPL, spl)
        REG(BPL, bpl)
        REG(SIL, sil)
        REG(DIL, dil)
        REG(R8B, r8b)
        REG(R9B, r9b)
        REG(R10B, r10b)
        REG(R11B, r11b)
        REG(R12B, r12b)
        REG(R13B, r13b)
        REG(R14B, r14b)
        REG(R15B, r15b)
        REG(AX, ax)
        REG(CX, cx)
        REG(DX, dx)
        REG(BX, bx)
        REG(SP, sp)
        REG(BP, bp)
        REG(SI, si)
        REG(DI, di)
        REG(R8W, r8w)
        REG(R9W, r9w)
        REG(R10W, r10w)
        REG(R11W, r11w)
        REG(R12W, r12w)
        REG(R13W, r13w)
        REG(R14W, r14w)
        REG(R15W, r15w)
        REG(EAX, eax)
        REG(ECX, ecx)
        REG(EDX, edx)
        REG(EBX, ebx)
        REG(ESP, esp)
        REG(EBP, ebp)
        REG(ESI, esi)
        REG(EDI, edi)
        REG(R8D, r8d)
        REG(R9D, r9d)
        REG(R10D, r10d)
        REG(R11D, r11d)
        REG(R12D, r12d)
        REG(R13D, r13d)
        REG(R14D, r14d)
        REG(R15D, r15d)
        REG(RAX, rax)
        REG(RCX, rcx)
        REG(RDX, rdx)
        REG(RBX, rbx)
        REG(RSP, rsp)
        REG(RBP, rbp)
        REG(RSI, rsi)
        REG(RDI, rdi)
        REG(R8, r8)
        REG(R9, r9)
        REG(R10, r10)
        REG(R11, r11)
        REG(R12, r12)
        REG(R13, r13)
        REG(R14, r14)
        REG(R15, r15)
        REG(ES, es)
        REG(CS, cs)
        REG(SS, ss)
        REG(DS, ds)
        REG(FS, fs)
        REG(GS, gs)
        REG(CR0, cr0)
        REG(CR1, cr1)
        REG(CR2, cr2)
        REG(CR3, cr3)
        REG(CR4, cr4)
        REG(CR5, cr5)
        REG(CR6, cr6)
        REG(CR7, cr7)
        REG(CR8, cr8)
        REG(CR9, cr9)
        REG(CR10, cr10)
        REG(CR11, cr11)
        REG(CR12, cr12)
        REG(CR13, cr13)
        REG(CR14, cr14)
        REG(CR15, cr15)
        REG(DR0, dr0)
        REG(DR1, dr1)
        REG(DR2, dr2)
        REG(DR3, dr3)
        REG(DR4, dr4)
        REG(DR5, dr5)
        REG(DR6, dr6)
        REG(DR7, dr7)
        REG(MM0, mm0)
        REG(MM1, mm1)
        REG(MM2, mm2)
        REG(MM3, mm3)
        REG(MM4, mm4)
        REG(MM5, mm5)
        REG(MM6, mm6)
        REG(MM7, mm7)
        REG(ST0, st0)
        REG(ST1, st1)
        REG(ST2, st2)
        REG(ST3, st3)
        REG(ST4, st4)
        REG(ST5, st5)
        REG(ST6, st6)
        REG(ST7, st7)
        REG(XMM0, xmm0)
        REG(XMM1, xmm1)
        REG(XMM2, xmm2)
        REG(XMM3, xmm3)
        REG(XMM4, xmm4)
        REG(XMM5, xmm5)
        REG(XMM6, xmm6)
        REG(XMM7, xmm7)
        REG(XMM8, xmm8)
        REG(XMM9, xmm9)
        REG(XMM10, xmm10)
        REG(XMM11, xmm11)
        REG(XMM12, xmm12)
        REG(XMM13, xmm13)
        REG(XMM14, xmm14)
        REG(XMM15, xmm15)
        case X86_REG_EIP: return std::make_unique<core::ir::Constant>(
            SizedValue(X86Registers::eip()->size(), instruction_->endAddr()));
        case X86_REG_RIP: return std::make_unique<core::ir::Constant>(
            SizedValue(X86Registers::rip()->size(), instruction_->endAddr()));

        #undef REG
        #undef REG_ST

        default:
            unreachable();
        }
    }

    std::unique_ptr<core::ir::Dereference> createDereference(const cs_x86_op &operand) const {
        return std::make_unique<core::ir::Dereference>(
            createDereferenceAddress(operand), core::ir::MemoryDomain::MEMORY, operand.size * CHAR_BIT);
    }

    std::unique_ptr<core::ir::Term> createDereferenceAddress(const cs_x86_op &operand) const {
        if (operand.type != X86_OP_MEM) {
            throw core::irgen::InvalidInstructionException(tr("Expected the operand to be a memory operand"));
        }

        std::unique_ptr<core::ir::Term> result = createRegisterAccess(operand.mem.base);

        if (operand.mem.scale != 0) {
            if (auto index = createRegisterAccess(operand.mem.index)) {
                if (operand.mem.scale != 1) {
                    index = std::make_unique<core::ir::BinaryOperator>(
                        core::ir::BinaryOperator::MUL,
                        std::move(index),
                        std::make_unique<core::ir::Constant>(SizedValue(addressSize(), operand.mem.scale)),
                        addressSize());
                }
                if (result) {
                    result = std::make_unique<core::ir::BinaryOperator>(
                        core::ir::BinaryOperator::ADD, std::move(result), std::move(index), addressSize());
                } else {
                    result = std::move(index);
                }
            }
        }

        auto offsetValue = SizedValue(addressSize(), operand.mem.disp);

        if (offsetValue.value() || !result) {
            auto offset = std::make_unique<core::ir::Constant>(offsetValue);

            if (result) {
                result = std::make_unique<core::ir::BinaryOperator>(
                    core::ir::BinaryOperator::ADD, std::move(result), std::move(offset), addressSize());
            } else {
                result = std::move(offset);
            }
        }

        return result;
    }

    /**
     * \return Default operand size, inferred from the execution mode and instruction prefixes.
     */
    SmallBitSize operandSize() const {
        if (architecture_->bitness() == 16) {
            return detail_->prefix[2] == X86_PREFIX_OPSIZE ? 32 : 16;
        } else if (architecture_->bitness() == 32) {
            return detail_->prefix[2] == X86_PREFIX_OPSIZE ? 16 : 32;
        } else if (architecture_->bitness() == 64) {
            if (detail_->rex) {
                return 64;
            } else {
                return detail_->prefix[2] == X86_PREFIX_OPSIZE ? 16 : 32;
            }
        } else {
            unreachable();
        }
    }

    /**
     * \return Default operand size, inferred from the execution mode and instruction prefixes.
     */
    SmallBitSize addressSize() const {
        return detail_->addr_size * CHAR_BIT;
    }
};

X86InstructionAnalyzer::X86InstructionAnalyzer(const X86Architecture *architecture):
    impl_(std::make_unique<X86InstructionAnalyzerImpl>(architecture))
{}

X86InstructionAnalyzer::~X86InstructionAnalyzer() {}

void X86InstructionAnalyzer::doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) {
    impl_->createStatements(checked_cast<const X86Instruction *>(instruction), program);
}

} // namespace x86
} // namespace arch
} // namespace nc

/* vim:set et sts=4 sw=4: */
