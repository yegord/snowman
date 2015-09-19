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

#include <boost/range/size.hpp>

#include <nc/common/CheckedCast.h>
#include <nc/common/Unreachable.h>

#include <nc/core/ir/Program.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/irgen/Expressions.h>
#include <nc/core/irgen/InvalidInstructionException.h>

#include "X86Architecture.h"
#include "X86Instruction.h"
#include "X86Registers.h"

#include "udis86.h"

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
    ud_t ud_obj_;
    const X86Instruction *currentInstruction_;

public:
    explicit
    X86InstructionAnalyzerImpl(const X86Architecture *architecture):
        architecture_(architecture)
    {
        assert(architecture != nullptr);

        ud_init(&ud_obj_);
        ud_set_mode(&ud_obj_, architecture_->bitness());
    }

    void createStatements(const X86Instruction *instr, core::ir::Program *program) {
        assert(instr != nullptr);
        assert(program != nullptr);

        currentInstruction_ = instr;

        ud_set_pc(&ud_obj_, instr->addr());
        ud_set_input_buffer(&ud_obj_, const_cast<uint8_t *>(instr->bytes()), checked_cast<std::size_t>(instr->size()));
        ud_disassemble(&ud_obj_);

        assert(ud_obj_.mnemonic != UD_Iinvalid);

        core::ir::BasicBlock *cachedDirectSuccessor = nullptr;
        auto directSuccessor = [&]() -> core::ir::BasicBlock * {
            if (!cachedDirectSuccessor) {
                cachedDirectSuccessor = program->createBasicBlock(instr->endAddr());
            }
            return cachedDirectSuccessor;
        };

        X86ExpressionFactory factory(architecture_);
        X86ExpressionFactoryCallback _(factory, program->getBasicBlockForInstruction(instr), instr);

        using namespace core::irgen::expressions;

        /* Describing semantics */
        switch (ud_obj_.mnemonic) {
            case UD_Iadc: {
                _[
                    operand(0) ^= operand(0) + operand(1) + zero_extend(cf),
                    cf ^= intrinsic(),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= signed_(operand(0)) < constant(0),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Iadd: {
                _[
                    operand(0) ^= operand(0) + operand(1),
                    cf ^= intrinsic(),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= signed_(operand(0)) < constant(0),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Iand: {
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
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Ibound: {
                /* Deprecated, used mostly for debugging, it's better to generate no IR code at all. */
                break;
            }
            case UD_Ibt: {
                _[
                    cf ^= truncate(unsigned_(operand(0)) >> operand(1)),
                    of ^= undefined(),
                    sf ^= undefined(),
                    zf ^= undefined(),
                    af ^= undefined(),
                    pf ^= undefined(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Icall: {
                auto sp = architecture_->stackPointer();
                auto ip = architecture_->instructionPointer();

                _[
                    regizter(sp) ^= regizter(sp) - constant(ip->size() / CHAR_BIT),
                    *regizter(sp) ^= constant(instr->endAddr(), ip->size()),
                    call(operand(0)),
                    regizter(sp) ^= regizter(sp) + constant(ip->size() / CHAR_BIT)
                ];
                break;
            }
            case UD_Icbw: {
                _[
                    regizter(X86Registers::ax()) ^= sign_extend(regizter(X86Registers::al()))
                ];
                break;
            }
            case UD_Icwde: {
                _[
                    regizter(X86Registers::eax()) ^= sign_extend(regizter(X86Registers::ax()))
                ];
                break;
            }
            case UD_Icdqe: {
                _[
                    regizter(X86Registers::rax()) ^= sign_extend(regizter(X86Registers::eax()))
                ];
                break;
            }
            case UD_Icqo: {
                _[
                    regizter(X86Registers::rdx()) ^= signed_(regizter(X86Registers::rax())) >> constant(63)
                ];
                break;
            }
            case UD_Icld: {
                _[
                    df ^= constant(0)
                ];
                break;
            }
            case UD_Icmp: {
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
            case UD_Icmpsb: case UD_Icmpsw: case UD_Icmpsd: case UD_Icmpsq:
            case UD_Imovsb: case UD_Imovsw: case UD_Imovsd: case UD_Imovsq:
            case UD_Iscasb: case UD_Iscasw: case UD_Iscasd: case UD_Iscasq:
            case UD_Istosb: case UD_Istosw: case UD_Istosd: case UD_Istosq: {
                SmallBitSize accessSize;

                switch (ud_obj_.mnemonic) {
                    case UD_Icmpsb: case UD_Imovsb: case UD_Iscasb: case UD_Istosb:
                        accessSize = 8;
                        break;
                    case UD_Icmpsw: case UD_Imovsw: case UD_Iscasw: case UD_Istosw:
                        accessSize = 16;
                        break;
                    case UD_Icmpsd: case UD_Imovsd: case UD_Iscasd: case UD_Istosd:
                        accessSize = 32;
                        break;
                    case UD_Icmpsq: case UD_Imovsq: case UD_Iscasq: case UD_Istosq:
                        accessSize = 64;
                        break;
                    default:
                        unreachable();
                }

                auto di = resizedRegister(X86Registers::di(), ud_obj_.adr_mode);
                auto si = resizedRegister(X86Registers::si(), ud_obj_.adr_mode);
                auto cx = resizedRegister(X86Registers::cx(), ud_obj_.opr_mode);

                auto increment = temporary(ud_obj_.adr_mode);
                _[
                    increment ^= constant(accessSize / CHAR_BIT) - constant(2 * accessSize / CHAR_BIT, si.memoryLocation().size()) * zero_extend(df)
                ];

                X86ExpressionFactoryCallback condition(factory, program->createBasicBlock(), instr);
                X86ExpressionFactoryCallback body(factory, program->createBasicBlock(), instr);

                _[jump(condition.basicBlock())];

                if (ud_obj_.pfx_rep != UD_NONE || ud_obj_.pfx_repe != UD_NONE || ud_obj_.pfx_repne != UD_NONE) {
                    condition[jump(cx, body.basicBlock(), directSuccessor())];

                    body[cx ^= cx - constant(1)];
                } else {
                    condition[jump(body.basicBlock())];
                }

                bool repPrefixIsValid = false;
                switch (ud_obj_.mnemonic) {
                    case UD_Icmpsb: case UD_Icmpsw: case UD_Icmpsd: case UD_Icmpsq: {
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
                    }
                    case UD_Imovsb: case UD_Imovsw: case UD_Imovsd: case UD_Imovsq: {
                        repPrefixIsValid = true;
                        body[dereference(di, accessSize) ^= *si];
                        break;
                    }
                    case UD_Iscasb: case UD_Iscasw: case UD_Iscasd: case UD_Iscasq: {
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
                        break;
                    }
                    case UD_Istosb: case UD_Istosw: case UD_Istosd: case UD_Istosq: {
                        repPrefixIsValid = true;
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

                /* libudis86 sets REP prefix together with REPZ/REPNZ. */
                if (ud_obj_.pfx_rep != UD_NONE && repPrefixIsValid) {
                    body[jump(condition.basicBlock())];
                } else if (ud_obj_.pfx_repe != UD_NONE) {
                    body[jump(zf, condition.basicBlock(), directSuccessor())];
                } else if (ud_obj_.pfx_repne != UD_NONE) {
                    body[jump(~zf, condition.basicBlock(), directSuccessor())];
                } else {
                    body[jump(directSuccessor())];
                }
                break;
            }
            case UD_Icmpxchg: {
                X86ExpressionFactoryCallback then(factory, program->createBasicBlock(), instr);

                _[
                    cf ^= intrinsic(),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == operand(1),
                    sf ^= intrinsic(),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf,

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
            case UD_Icpuid: {
                _[
                    regizter(X86Registers::eax()) ^= intrinsic(),
                    regizter(X86Registers::ebx()) ^= intrinsic(),
                    regizter(X86Registers::ecx()) ^= intrinsic(),
                    regizter(X86Registers::edx()) ^= intrinsic()
                ];
                break;
            }
            case UD_Idec: {
                _[
                    operand(0) ^= operand(0) - constant(1),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= signed_(operand(0)) < constant(0),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Idiv: {
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
            case UD_Ihlt: {
                _(std::make_unique<core::ir::InlineAssembly>());
                _[halt()];
                break;
            }
            case UD_Iidiv: {
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
            case UD_Iint: {
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
            case UD_Iint3: {
                /* int 3 is a debug break, remove it. */
                break;
            }
            case UD_Iimul: case UD_Imul: {
                /* All compilers always use IMUL, not MUL. Even for unsigned operands.
                 * See http://stackoverflow.com/questions/4039378/x86-mul-instruction-from-vs-2008-2010
                 *
                 * So, there is no such thing as signed or unsigned multiplication.
                 */
                if (!hasOperand(1)) {
                    /* One operand. */
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
                } else if (!hasOperand(2)) {
                    /* Two operands. */
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
                    pf ^= undefined(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Iinc: {
                _[
                    operand(0) ^= operand(0) + constant(1),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= signed_(operand(0)) < constant(0),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Ija: {
                _[jump(~below_or_equal, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijae: {
                _[jump(~cf, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijb: {
                _[jump(cf, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijbe: {
                _[jump(below_or_equal, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijcxz: {
                _[jump(~(cx == constant(0)), operand(0), directSuccessor())];
                break;
            }
            case UD_Ijecxz: {
                _[jump(~(ecx == constant(0)), operand(0), directSuccessor())];
                break;
            }
            case UD_Ijrcxz: {
                _[jump(~(rcx == constant(0)), operand(0), directSuccessor())];
                break;
            }
            case UD_Ijz: {
                _[jump(zf, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijg: {
                _[jump(~less_or_equal, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijge: {
                _[jump(~less, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijl: {
                _[jump(less, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijle: {
                _[jump(less_or_equal, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijnz: {
                _[jump(~zf, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijno: {
                _[jump(~of, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijnp: {
                _[jump(~pf, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijns: {
                _[jump(~sf, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijo: {
                _[jump(of, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijp: {
                _[jump(pf, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijs: {
                _[jump(sf, operand(0), directSuccessor())];
                break;
            }
            case UD_Ijmp: {
                _[jump(operand(0))];
                break;
            }
            case UD_Icmova: case UD_Icmovae: case UD_Icmovb: case UD_Icmovbe:
            case UD_Icmovz: case UD_Icmovg: case UD_Icmovge: case UD_Icmovl:
            case UD_Icmovle: case UD_Icmovnz: case UD_Icmovno: case UD_Icmovnp:
            case UD_Icmovns: case UD_Icmovo: case UD_Icmovp: case UD_Icmovs: {
                X86ExpressionFactoryCallback then(factory, program->createBasicBlock(), instr);

                switch (ud_obj_.mnemonic) {
                    case UD_Icmova:
                        _[jump(~below_or_equal, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovae:
                        _[jump(~cf, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovb:
                        _[jump(cf, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovbe:
                        _[jump(below_or_equal, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovz:
                        _[jump(zf, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovg:
                        _[jump(~less_or_equal, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovge:
                        _[jump(~less, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovl:
                        _[jump(less, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovle:
                        _[jump(less_or_equal, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovnz:
                        _[jump(~zf, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovno:
                        _[jump(~of, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovnp:
                        _[jump(~pf, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovns:
                        _[jump(~sf, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovo:
                        _[jump(of, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovp:
                        _[jump(pf, then.basicBlock(), directSuccessor())]; break;
                    case UD_Icmovs:
                        _[jump(sf, then.basicBlock(), directSuccessor())]; break;
                    default: unreachable();
                }

                then[
                    operand(0) ^= operand(1),
                    jump(directSuccessor())
                ];

                break;
            }
            case UD_Ilea: {
                auto operand0 = operand(0);
                auto operand1 = core::irgen::expressions::TermExpression(createDereferenceAddress(ud_obj_.operand[1]));

                if (operand0.size() == operand1.size()) {
                    _[std::move(operand0) ^= std::move(operand1)];
                } else if (operand0.size() < operand1.size()) {
                    _[std::move(operand0) ^= truncate(std::move(operand1))];
                } else if (operand0.size() > operand1.size()) {
                    _[std::move(operand0) ^= zero_extend(std::move(operand1))];
                }
                break;
            }
            case UD_Ileave: {
                auto sp = architecture_->stackPointer();
                auto bp = architecture_->basePointer();

                _[
                    regizter(sp) ^= regizter(bp),
                    regizter(bp) ^= *regizter(sp),
                    regizter(sp) ^= regizter(sp) + constant(bp->size() / CHAR_BIT)
                ];
                break;
            }
            case UD_Iloop:
            case UD_Iloope:
            case UD_Iloopnz: {
                auto cx = resizedRegister(X86Registers::cx(), ud_obj_.adr_mode);

                _[cx ^= cx - constant(1)];

                switch (ud_obj_.mnemonic) {
                    case UD_Iloop:
                        _[jump(~(cx == constant(0)), operand(0), directSuccessor())];
                        break;
                    case UD_Iloope:
                        _[jump(~(cx == constant(0)) & zf, operand(0), directSuccessor())];
                        break;
                    case UD_Iloopnz:
                        _[jump(~(cx == constant(0)) & ~zf, operand(0), directSuccessor())];
                        break;
                    default:
                        unreachable();
                }
                break;
            }
            case UD_Imov: {
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
            case UD_Imovsx: case UD_Imovsxd: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);

                if (operand0.size() == operand1.size()) {
                    _[std::move(operand0) ^= std::move(operand1)];
                } else if (operand0.size() > operand1.size()) {
                    _[std::move(operand0) ^= sign_extend(std::move(operand1))];
                } else {
                    /* Weird, but observed in real life. */
                    _[std::move(operand0) ^= truncate(std::move(operand1))];
                }
                break;
            }
            case UD_Imovzx: {
                _[operand(0) ^= zero_extend(operand(1))];
                break;
            }
            case UD_Ineg: {
                _[
                    cf ^= ~(operand(0) == constant(0)),
                    operand(0) ^= -operand(0),
                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0),
                    sf ^= intrinsic(),
                    of ^= intrinsic(),
                    af ^= intrinsic(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Inop: {
                break;
            }
            case UD_Inot: {
                _[operand(0) ^= ~operand(0)];
                break;
            }
            case UD_Ior: {
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
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Ipop: {
                auto sp = architecture_->stackPointer();
                auto operand0 = operand(0);
                auto size = operand0.size();
                _[
                    std::move(operand0) ^= *regizter(sp),
                    regizter(sp) ^= regizter(sp) + constant(size / CHAR_BIT)
                ];
                break;
            }
            case UD_Ipush: {
                auto sp = architecture_->stackPointer();
                const auto &op = ud_obj_.operand[0];
                if (op.type == UD_OP_IMM) {
                    _[
                        regizter(sp) ^= regizter(sp) - constant(sp->size() / CHAR_BIT),
                        *regizter(sp) ^= constant(getSignedValue(op, sp->size()), sp->size())
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
            case UD_Ipopfw: {
                auto sp = architecture_->stackPointer();
                _[
                    *regizter(sp) ^= flags,
                    regizter(sp) ^= regizter(sp) + constant(2)
                ];
                break;
            }
            case UD_Ipopfd: {
                auto sp = architecture_->stackPointer();
                _[
                    eflags ^= *regizter(sp),
                    regizter(sp) ^= regizter(sp) + constant(4)
                ];
                break;
            }
            case UD_Ipopfq: {
                auto sp = architecture_->stackPointer();
                _[
                    rflags ^= *regizter(sp),
                    regizter(sp) ^= regizter(sp) + constant(8)
                ];
                break;
            }
            case UD_Ipushfw: {
                auto sp = architecture_->stackPointer();
                _[
                    regizter(sp) ^= regizter(sp) - constant(2),
                    *regizter(sp) ^= flags
                ];
                break;
            }
            case UD_Ipushfd: {
                auto sp = architecture_->stackPointer();
                _[
                    regizter(sp) ^= regizter(sp) - constant(4),
                    *regizter(sp) ^= eflags & constant(0x00fcffff)
                ];
                break;
            }
            case UD_Ipushfq: {
                auto sp = architecture_->stackPointer();
                _[
                    regizter(sp) ^= regizter(sp) - constant(8),
                    *regizter(sp) ^= rflags & constant(0x00fcffff)
                ];
                break;
            }
            case UD_Iret: {
                auto sp = architecture_->stackPointer();
                auto ip = architecture_->instructionPointer();

                _[regizter(ip) ^= *regizter(sp)];
                /* sp is incremented in call instruction. */

                if (hasOperand(0)) {
                    _[regizter(sp) ^= regizter(sp) + zero_extend(operand(0))];
                }

                _[jump(regizter(ip))];

                break;
            }
            case UD_Ishl: {
                if (hasOperand(1)) {
                    _[operand(0) ^= operand(0) << operand(1)];
                } else {
                    _[operand(0) ^= operand(0) << constant(1)];
                }

                _[
                    cf ^= intrinsic(),
                    sf ^= undefined(),
                    zf ^= operand(0) == constant(0),
                    pf ^= intrinsic(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Isar: {
                if (hasOperand(1)) {
                    _[operand(0) ^= signed_(operand(0)) >> operand(1)];
                } else {
                    _[operand(0) ^= signed_(operand(0)) >> constant(1)];
                }

                _[
                    cf ^= intrinsic(),
                    sf ^= undefined(),
                    zf ^= operand(0) == constant(0),
                    pf ^= intrinsic(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Isbb: {
                _[
                    less             ^= signed_(operand(0))   <  operand(1) + zero_extend(cf),
                    less_or_equal    ^= signed_(operand(0))   <= operand(1) + zero_extend(cf),
                    cf               ^= unsigned_(operand(0)) <  operand(1) + zero_extend(cf),
                    below_or_equal   ^= unsigned_(operand(0)) <= operand(1) + zero_extend(cf),

                    operand(0) ^= operand(0) - (operand(1) + zero_extend(cf)),

                    pf ^= intrinsic(),
                    zf ^= operand(0) == constant(0) + zero_extend(cf),
                    sf ^= signed_(operand(0)) < constant(0) + zero_extend(cf),
                    of ^= intrinsic(),
                    af ^= intrinsic()
                ];
                break;
            }
            case UD_Iseta: {
                _[operand(0) ^= zero_extend(~below_or_equal)];
                break;
            }
            case UD_Isetnb: {
                _[operand(0) ^= zero_extend(~cf)];
                break;
            }
            case UD_Isetb: {
                _[operand(0) ^= zero_extend(cf)];
                break;
            }
            case UD_Isetbe: {
                _[operand(0) ^= zero_extend(below_or_equal)];
                break;
            }
            case UD_Isetz: {
                _[operand(0) ^= zero_extend(zf)];
                break;
            }
            case UD_Isetg: {
                _[operand(0) ^= zero_extend(~less_or_equal)];
                break;
            }
            case UD_Isetge: {
                _[operand(0) ^= zero_extend(~less)];
                break;
            }
            case UD_Isetl: {
                _[operand(0) ^= zero_extend(less)];
                break;
            }
            case UD_Isetle: {
                _[operand(0) ^= zero_extend(less_or_equal)];
                break;
            }
            case UD_Isetnz: {
                _[operand(0) ^= zero_extend(~zf)];
                break;
            }
            case UD_Isetno: {
                _[operand(0) ^= zero_extend(~of)];
                break;
            }
            case UD_Isetnp: {
                _[operand(0) ^= zero_extend(~pf)];
                break;
            }
            case UD_Isetns: {
                _[operand(0) ^= zero_extend(~sf)];
                break;
            }
            case UD_Iseto: {
                _[operand(0) ^= zero_extend(of)];
                break;
            }
            case UD_Isetp: {
                _[operand(0) ^= zero_extend(pf)];
                break;
            }
            case UD_Isets: {
                _[operand(0) ^= zero_extend(sf)];
                break;
            }
            case UD_Ishr: {
                if (hasOperand(1)) {
                    _[operand(0) ^= unsigned_(operand(0)) >> operand(1)];
                } else {
                    _[operand(0) ^= unsigned_(operand(0)) >> constant(1)];
                }

                _[
                    cf ^= intrinsic(),
                    sf ^= undefined(),
                    zf ^= operand(0) == constant(0),
                    pf ^= intrinsic(),
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Istd: {
                _[
                    df ^= constant(1)
                ];
                break;
            }
            case UD_Isub: {
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
            case UD_Itest: {
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
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
                ];
                break;
            }
            case UD_Ixchg: {
                auto operand0 = operand(0);
                auto tmp = temporary(operand0.size());

                _[
                    tmp ^= operand(0),
                    operand(0) ^= operand(1),
                    operand(1) ^= tmp
                ];
                break;
            }
            case UD_Ixor: {
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
                    less ^= ~(sf == of),
                    less_or_equal ^= less | zf,
                    below_or_equal ^= cf | zf
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
    bool hasOperand(std::size_t index) const {
        assert(index < boost::size(ud_obj_.operand));
        return ud_obj_.operand[index].type != UD_NONE;
    }

    core::irgen::expressions::TermExpression operand(std::size_t index) const {
        return core::irgen::expressions::TermExpression(createTermForOperand(index));
    }

    bool operandsAreTheSame(std::size_t index1, std::size_t index2) const {
        assert(index1 < boost::size(ud_obj_.operand));
        assert(index2 < boost::size(ud_obj_.operand));

        const ud_operand &op1 = ud_obj_.operand[index1];
        const ud_operand &op2 = ud_obj_.operand[index2];

        return op1.type == UD_OP_REG && op2.type == UD_OP_REG && op1.base == op2.base;
    }

    std::unique_ptr<core::ir::Term> createTermForOperand(std::size_t index) const {
        assert(index < boost::size(ud_obj_.operand));

        const ud_operand &operand = ud_obj_.operand[index];

        switch (operand.type) {
            case UD_NONE:
                throw core::irgen::InvalidInstructionException(tr("The instruction does not have an argument with index %1").arg(index));
            case UD_OP_MEM:
                return createDereference(operand);
            case UD_OP_PTR:
                return std::make_unique<core::ir::Constant>(
                    SizedValue(operand.size, operand.lval.ptr.seg * 16 + operand.lval.ptr.off));
            case UD_OP_IMM: {
                /* Signed number, sign-extended to match the size of the other operand. */
                auto operandSize = operand.size;
                if (index > 0) {
                    operandSize = std::max(operandSize, ud_obj_.operand[index - 1].size);
                }
                return std::make_unique<core::ir::Constant>(SizedValue(operandSize, getSignedValue(operand, operand.size)));
            }
            case UD_OP_JIMM:
                return std::make_unique<core::ir::Constant>(
                    SizedValue(architecture_->bitness(), ud_obj_.pc + getSignedValue(operand, operand.size)));
            case UD_OP_CONST:
                /* This is some small constant value, like in "sar eax, 1". Its size is always zero. */
                assert(operand.size == 0);
                return std::make_unique<core::ir::Constant>(SizedValue(8, operand.lval.ubyte));
            case UD_OP_REG: {
                auto result = createRegisterAccess(operand.base);
                assert(result != nullptr);
                return result;
            }
            default:
                unreachable();
        }
    }

    SignedConstantValue getUnsignedValue(const ud_operand &operand, SmallBitSize size) const {
        switch (size) {
            case 0:  return 0;
            case 8:  return operand.lval.ubyte;
            case 16: return operand.lval.uword;
            case 32: return operand.lval.udword;
            case 64: return operand.lval.uqword;
            default: unreachable();
        }
    }

    SignedConstantValue getSignedValue(const ud_operand &operand, SmallBitSize size) const {
        switch (size) {
            case 0:  return 0;
            case 8:  return operand.lval.sbyte;
            case 16: return operand.lval.sword;
            case 32: return operand.lval.sdword;
            case 64: return operand.lval.sqword;
            default: unreachable();
        }
    }

    std::unique_ptr<core::ir::Term> createRegisterAccess(enum ud_type type) const {
        switch (type) {
        case UD_NONE: return nullptr;

        #define REG(ud_name, nc_name) case UD_R_##ud_name: return X86InstructionAnalyzer::createTerm(X86Registers::nc_name());

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
        REG(DR8, dr8)
        REG(DR9, dr9)
        REG(DR10, dr10)
        REG(DR11, dr11)
        REG(DR12, dr12)
        REG(DR13, dr13)
        REG(DR14, dr14)
        REG(DR15, dr15)
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
        case UD_R_RIP: return std::make_unique<core::ir::Constant>(
            SizedValue(X86Registers::rip()->size(), currentInstruction_->endAddr()));

        #undef REG
        #undef REG_ST

        default:
            unreachable();
        }
    }

    std::unique_ptr<core::ir::Dereference> createDereference(const ud_operand &operand) const {
        return std::make_unique<core::ir::Dereference>(
            createDereferenceAddress(operand), core::ir::MemoryDomain::MEMORY, operand.size);
    }

    std::unique_ptr<core::ir::Term> createDereferenceAddress(const ud_operand &operand) const {
        if (operand.type != UD_OP_MEM) {
            throw core::irgen::InvalidInstructionException(tr("Expected the operand to be a memory operand"));
        }

        std::unique_ptr<core::ir::Term> result = createRegisterAccess(operand.base);

        if (auto index = createRegisterAccess(operand.index)) {
            /* Scale can be 1, 2, 4, 8, or 0 (which means 1). */

            assert(operand.scale == 1 || operand.scale == 2 ||
                operand.scale == 4 || operand.scale == 8 || operand.scale == 0);

            if (operand.scale > 1) {
                index = std::make_unique<core::ir::BinaryOperator>(
                    core::ir::BinaryOperator::MUL,
                    std::move(index),
                    std::make_unique<core::ir::Constant>(SizedValue(ud_obj_.adr_mode, operand.scale)),
                    ud_obj_.adr_mode);
            }
            if (result) {
                result = std::make_unique<core::ir::BinaryOperator>(
                    core::ir::BinaryOperator::ADD, std::move(result), std::move(index), ud_obj_.adr_mode);
            } else {
                result = std::move(index);
            }
        }

        auto offsetValue = SizedValue(ud_obj_.adr_mode, getSignedValue(operand, operand.offset));

        if (offsetValue.value() || !result) {
            auto offset = std::make_unique<core::ir::Constant>(offsetValue);

            if (result) {
                result = std::make_unique<core::ir::BinaryOperator>(
                    core::ir::BinaryOperator::ADD, std::move(result), std::move(offset), ud_obj_.adr_mode);
            } else {
                result = std::move(offset);
            }
        }

        return result;
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
