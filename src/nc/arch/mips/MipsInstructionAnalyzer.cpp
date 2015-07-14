/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "MipsInstructionAnalyzer.h"

#include <QCoreApplication>

#include <boost/range/size.hpp>

#include <nc/common/CheckedCast.h>
#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Capstone.h>
#include <nc/core/ir/Program.h>
#include <nc/core/irgen/Expressions.h>
#include <nc/core/irgen/InvalidInstructionException.h>

#include <nc/core/arch/Instructions.h>

#include "MipsArchitecture.h"
#include "MipsInstruction.h"
#include "MipsRegisters.h"

namespace nc {
namespace arch {
namespace mips {

namespace {

class MipsExpressionFactory: public core::irgen::expressions::ExpressionFactory<MipsExpressionFactory> {
public:
    MipsExpressionFactory(const core::arch::Architecture *architecture):
        core::irgen::expressions::ExpressionFactory<MipsExpressionFactory>(architecture)
    {}
};

typedef core::irgen::expressions::ExpressionFactoryCallback<MipsExpressionFactory> MipsExpressionFactoryCallback;

NC_DEFINE_REGISTER_EXPRESSION(MipsRegisters, sp)
NC_DEFINE_REGISTER_EXPRESSION(MipsRegisters, gp)
NC_DEFINE_REGISTER_EXPRESSION(MipsRegisters, hilo)
NC_DEFINE_REGISTER_EXPRESSION(MipsRegisters, lo)

} // anonymous namespace

class MipsInstructionAnalyzerImpl {
    Q_DECLARE_TR_FUNCTIONS(MipsInstructionAnalyzerImpl)

    const MipsArchitecture *architecture_;
    core::arch::Capstone capstone_;
    MipsExpressionFactory factory_;
    core::ir::Program *program_;
    const MipsInstruction *instruction_;
    //core::arch::CapstoneInstructionPtr instr_;
    const cs_mips *detail_;
    const core::arch::Instructions *instructions_;

public:
    MipsInstructionAnalyzerImpl(const MipsArchitecture *architecture):
        architecture_(architecture), capstone_(CS_ARCH_MIPS, CS_MODE_MIPS32), factory_(architecture)
    {
        assert(architecture_ != nullptr);
    }

    void setInstructions(const core::arch::Instructions *instructions) {
        instructions_ = instructions;
    }

    void createStatements(MipsExpressionFactoryCallback & _, const MipsInstruction *instruction, core::ir::Program *program) {

        auto instr = disassemble(instruction);
        if (instr == nullptr)
            return;
       
        detail_ = &instr->detail->mips;
                
        core::ir::BasicBlock *cachedDirectSuccessor = nullptr;
        core::ir::BasicBlock *cachedNextDirectSuccessor = nullptr;
        auto directSuccessor = [&]() -> core::ir::BasicBlock * {
            if (!cachedDirectSuccessor) {
                cachedDirectSuccessor = program->createBasicBlock(instruction->endAddr());
            }
            return cachedDirectSuccessor;
        };
        auto directSuccessorButOne = [&]() -> core::ir::BasicBlock * {
            if (!cachedNextDirectSuccessor) {
                cachedNextDirectSuccessor = program->createBasicBlock(instruction->endAddr() + instruction->size());
            }
            return cachedNextDirectSuccessor;
        };

        auto delayslot = [&](MipsExpressionFactoryCallback & callback) -> MipsExpressionFactoryCallback & {
            auto detail = detail_;
            auto delayslot = checked_cast<const MipsInstruction *>(instructions_->get(instruction->endAddr()).get());
            if (delayslot) {
                createStatements(callback, delayslot, program);
            }
            else {
                throw core::irgen::InvalidInstructionException(tr("Cannot find a delay slot at 0x%1.").arg(instruction->endAddr(), 0, 16));
            }
            detail_ = detail;
            return callback;
        };

        auto directSuccessorAddress = instruction->endAddr();
        auto nextDirectSuccessorAddress = directSuccessorAddress + instruction->size();

        MipsExpressionFactory factory(architecture_);

        using namespace core::irgen::expressions;

        auto op_count = detail_->op_count;

        /* Describing semantics */
        switch (instr->id) {
            case MIPS_INS_CACHE: /* Fall-through */
            case MIPS_INS_BREAK:
            case MIPS_INS_PREF:
            case MIPS_INS_SYNC:
            case MIPS_INS_SSNOP:
            case MIPS_INS_NOP: {
                break;
            }
            case MIPS_INS_ABS: {
                MipsExpressionFactoryCallback then(factory, program->createBasicBlock(), instruction);
                _[	
                    jump((signed_(operand(1)) < signed_(constant(0))),
                         (then[operand(0) ^= -operand(1), jump(directSuccessor())]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_ADD: /* Fall-through */
            case MIPS_INS_ADDU: {
                _[operand(0) ^= (operand(1) + operand(2))];
                break;
            }
            case MIPS_INS_SUB: /* Fall-through */
            case MIPS_INS_SUBU: {
                _[operand(0) ^= (operand(1) - operand(2))];
                break;
            }
            case MIPS_INS_NEG: /* Fall-through */
            case MIPS_INS_NEGU: {
                _[operand(0) ^= (constant(0) - operand(1))];
                break;
            }
            case MIPS_INS_AND: {
                _[operand(0) ^= (operand(1) & operand(2))];
                break;
            }
            case MIPS_INS_OR: {
                _[operand(0) ^= (operand(1) | operand(2))];
                break;
            }
            case MIPS_INS_XOR: {
                _[operand(0) ^= (operand(1) ^ operand(2))];
                break;
            }
            case MIPS_INS_NOR: {
                _[operand(0) ^= ~(operand(1) | operand(2))];
                break;
            }
            case MIPS_INS_NOT: {
                _[operand(0) ^= ~operand(1)];
                break;
            }
            case MIPS_INS_ADDI: /* Fall-through */
            case MIPS_INS_ADDIU: {
                _[operand(0) ^= (operand(1) + signed_(operand(2)))];
                break;
            }
            case MIPS_INS_ANDI: {
                _[operand(0) ^= (operand(1) & unsigned_(operand(2)))];
                break;
            }
            case MIPS_INS_ORI: {
                _[operand(0) ^= (operand(1) | unsigned_(operand(2)))];
                break;
            }
            case MIPS_INS_XORI: {
                _[operand(0) ^= (operand(1) ^ unsigned_(operand(2)))];
                break;
            }
            case MIPS_INS_LUI: {
                _[operand(0) ^= (operand(1) << constant(16))];
                break;
            }
            case MIPS_INS_MOVE: {
                _[operand(0) ^= operand(1)];
                break;
            }
            case MIPS_INS_MOVN: {
                MipsExpressionFactoryCallback then(factory, program->createBasicBlock(), instruction);
                _[
                    jump(~(operand(2) == constant(0)),
                         (then[operand(0) ^= operand(1), jump(directSuccessor())]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_MOVZ: {
                MipsExpressionFactoryCallback then(factory, program->createBasicBlock(), instruction);
                _[
                    jump(operand(2) == constant(0),
                         (then[operand(0) ^= operand(1), jump(directSuccessor())]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_SEB: {
                _[operand(0) ^= sign_extend(operand(1, 8))];
                break;
            }
            case MIPS_INS_SEH: {
                _[operand(0) ^= sign_extend(operand(1, 16))];
                break;
            }
            case MIPS_INS_SEQ: {
                /* d = (s == t) ? 1 : 0 */
                MipsExpressionFactoryCallback _1(factory, program->createBasicBlock(), instruction);
                MipsExpressionFactoryCallback _0(factory, program->createBasicBlock(), instruction);
                _[
                    jump(operand(1) == operand(2),
                         _1[operand(0) ^= constant(1), jump(directSuccessor())].basicBlock(),
                         _0[operand(0) ^= constant(0), jump(directSuccessor())].basicBlock())
                ];
                break;
            }
            case MIPS_INS_SNE: {
                /* d = (s != t) ? 1 : 0 */
                MipsExpressionFactoryCallback _1(factory, program->createBasicBlock(), instruction);
                MipsExpressionFactoryCallback _0(factory, program->createBasicBlock(), instruction);
                _[
                    jump(~(operand(1) == operand(2)),
                         _1[operand(0) ^= constant(1), jump(directSuccessor())].basicBlock(),
                         _0[operand(0) ^= constant(0), jump(directSuccessor())].basicBlock())
                ];
                break;
            }
            case MIPS_INS_SEQI: {
                MipsExpressionFactoryCallback _1(factory, program->createBasicBlock(), instruction);
                MipsExpressionFactoryCallback _0(factory, program->createBasicBlock(), instruction);
                _[
                    jump(operand(1) == signed_(operand(2)),
                         _1[operand(0) ^= constant(1), jump(directSuccessor())].basicBlock(),
                         _0[operand(0) ^= constant(0), jump(directSuccessor())].basicBlock())
                ];
                break;
            }
            case MIPS_INS_SNEI: {
                /* d = (s != t) ? 1 : 0 */
                MipsExpressionFactoryCallback _1(factory, program->createBasicBlock(), instruction);
                MipsExpressionFactoryCallback _0(factory, program->createBasicBlock(), instruction);
                _[
                    jump(~(operand(1) == signed_(operand(2))),
                         _1[operand(0) ^= constant(1), jump(directSuccessor())].basicBlock(),
                         _0[operand(0) ^= constant(0), jump(directSuccessor())].basicBlock())
                ];
                break;
            }
            case MIPS_INS_SLT: /* Fall-through */
            case MIPS_INS_SLTI: {
                MipsExpressionFactoryCallback _1(factory, program->createBasicBlock(), instruction);
                MipsExpressionFactoryCallback _0(factory, program->createBasicBlock(), instruction);
                _[
                    jump((signed_(operand(1)) < signed_(operand(2))),
                         _1[operand(0) ^= constant(1), jump(directSuccessor())].basicBlock(),
                         _0[operand(0) ^= constant(0), jump(directSuccessor())].basicBlock())
                ];
                break;
            }
            case MIPS_INS_SLTU: /* Fall-through */
            case MIPS_INS_SLTIU: {
                MipsExpressionFactoryCallback _1(factory, program->createBasicBlock(), instruction);
                MipsExpressionFactoryCallback _0(factory, program->createBasicBlock(), instruction);
                _[
                    jump((unsigned_(operand(1)) < unsigned_(operand(2))),
                         _1[operand(0) ^= constant(1), jump(directSuccessor())].basicBlock(),
                         _0[operand(0) ^= constant(0), jump(directSuccessor())].basicBlock())
                ];
                break;
            }
            case MIPS_INS_ROTR:	 /* Fall-through */
            case MIPS_INS_ROTRV: {
                auto operand1 = operand(1);
                auto operand2 = operand(2);
                _[operand(0) ^= ((unsigned_(operand1) >> operand2) | (operand1 << (constant(32) - operand2)))];
                break;
            }
            case MIPS_INS_SLL: /* Fall-through */
            case MIPS_INS_SLLI:
            case MIPS_INS_SLLV: {
                if (getOperandType(2) == MIPS_OP_REG)
                    _[operand(0) ^= (operand(1) << zero_extend(operand(2, 4)))];
                else
                    _[operand(0) ^= (operand(1) << operand(2))];
                break;
            }
            case MIPS_INS_SRA: /* Fall-through */
            case MIPS_INS_SRAI:
            case MIPS_INS_SRAV: {
                if (getOperandType(2) == MIPS_OP_REG)
                    _[operand(0) ^= (signed_(operand(1)) >> zero_extend(operand(2, 4)))];
                else
                    _[operand(0) ^= (signed_(operand(1)) >> operand(2))];
                break;
            }
            case MIPS_INS_SRL: /* Fall-through */
            case MIPS_INS_SRLI:
            case MIPS_INS_SRLV: {
                if (getOperandType(2) == MIPS_OP_REG)
                    _[operand(0) ^= (unsigned_(operand(1)) >> zero_extend(operand(2, 4)))];
                else
                    _[operand(0) ^= (unsigned_(operand(1)) >> operand(2))];
                break;
            }

            case MIPS_INS_LB: {
                _[operand(0) ^= sign_extend(operand(1, 8))];
                break;
            }
            case MIPS_INS_LBU: {
                _[operand(0) ^= zero_extend(operand(1, 8))];
                break;
            }
            case MIPS_INS_LH: {
                _[operand(0) ^= sign_extend(operand(1, 16))];
                break;
            }
            case MIPS_INS_LHU: {
                _[operand(0) ^= zero_extend(operand(1, 16))];
                break;
            }
            case MIPS_INS_LW: {
                _[operand(0) ^= operand(1)];
                break;
            }
            case MIPS_INS_LWL: {
				auto operand0 = operand(0);
				_[operand0 ^= ((operand(0) & constant(0xffff)) | (operand(1) << constant(16)))];
                break;
            }
            case MIPS_INS_LWR: {
				auto operand0 = operand(0);
				_[operand0 ^= ((operand(0) & constant(0xffff0000)) | (operand(1)))];
                break;
            }
            case MIPS_INS_SB: {
                _[operand(0, 8) ^= truncate(operand(1))];
                break;
            }
            case MIPS_INS_SH: {
                _[operand(0, 16) ^= truncate(operand(1))];
                break;
            }
            case MIPS_INS_SW: {
                _[operand(0) ^= operand(1)];
                break;
            }
            case MIPS_INS_SWL: {
				auto operand0 = operand(0);
				_[operand0 ^= ((operand(0) & constant(0xffff)) | (operand(1) << constant(16)))];
                break;
            }
            case MIPS_INS_SWR: {
				auto operand0 = operand(0);
				_[operand0 ^= ((operand(0) & constant(0xffff0000)) | (operand(1)))];
                break;
            }
            case MIPS_INS_DIV: /* Fall-through */
            case MIPS_INS_DIVU: {
                if (op_count == 2)
                    _[
                        regizter(MipsRegisters::hi()) ^= unsigned_(operand(0)) % unsigned_(operand(1)),
                        regizter(MipsRegisters::lo()) ^= unsigned_(operand(0)) / unsigned_(operand(1))
                    ];
                else
                    _[
                        regizter(MipsRegisters::hi()) ^= unsigned_(operand(1)) % unsigned_(operand(2)),
                        regizter(MipsRegisters::lo()) ^= unsigned_(operand(1)) / unsigned_(operand(2)),
                       	operand(0) ^= regizter(MipsRegisters::lo())
                    ];
                break;
            }

            case MIPS_INS_MFHI: {
                auto operand0 = operand(0);
                _[
                    std::move(operand0) ^= regizter(MipsRegisters::hi())
                ];
                break;
            }
            case MIPS_INS_MTHI: {
                auto operand0 = operand(0);
                _[
                    regizter(MipsRegisters::hi()) ^= std::move(operand0)
                ];
                break;
            }
            case MIPS_INS_MFLO: {
                auto operand0 = operand(0);
                _[
                    std::move(operand0) ^= regizter(MipsRegisters::lo())
                ];
                break;
            }
            case MIPS_INS_MTLO: {
                auto operand0 = operand(0);
                _[
                    regizter(MipsRegisters::lo()) ^= std::move(operand0)
                ];
                break;
            }
#if 0
            /* FIXME: hi/lo */
            case MIPS_INS_MAD: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);
                _[
                    regizter(MipsRegisters::hilo()) ^= regizter(MipsRegisters::hilo()) + sign_extend((std::move(operand0) * std::move(operand1)), 64)
                ];
                break;
            }
            /* FIXME: hi/lo */
            case MIPS_INS_MADU: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);
                _[
                    regizter(MipsRegisters::hilo()) ^= regizter(MipsRegisters::hilo()) + zero_extend((std::move(operand0) * std::move(operand1)), 64)
                ];
                break;
            }
#endif
            /* FIXME: hi/lo */
            case MIPS_INS_MADD: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);
                auto operand2 = operand(2);
                _[
                    regizter(MipsRegisters::hilo()) ^= regizter(MipsRegisters::hilo()) + sign_extend((std::move(operand1) * std::move(operand2)), 64),
                    operand0 ^= regizter(MipsRegisters::lo()) 
                ];
                break;
            }
            /* FIXME: hi/lo */
            case MIPS_INS_MADDU: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);
                auto operand2 = operand(2);
                _[
                    regizter(MipsRegisters::hilo()) ^= regizter(MipsRegisters::hilo()) + zero_extend((std::move(operand1) * std::move(operand2)), 64),
                    operand0 ^= regizter(MipsRegisters::lo())
                ];
                break;
            }
            case MIPS_INS_MSUB: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);
                _[
                    regizter(MipsRegisters::hilo()) ^= regizter(MipsRegisters::hilo()) - sign_extend((std::move(operand0) * std::move(operand1)), 64)
                ];
                break;
            }
            case MIPS_INS_MSUBU: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);
                _[
                    regizter(MipsRegisters::hilo()) ^= regizter(MipsRegisters::hilo()) - zero_extend((std::move(operand0) * std::move(operand1)), 64)
                ];
                break;
            }
            /* FIXME: hi/lo */
            case MIPS_INS_MUL: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);
                auto operand2 = operand(2);
                _[
                    regizter(MipsRegisters::hilo()) ^= sign_extend((std::move(operand1) * std::move(operand2)), 64),
                    operand0 ^= regizter(MipsRegisters::lo())
                ];
                break;
            }
            /* FIXME: hi/lo */
            case MIPS_INS_MULT: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);
                _[
                    regizter(MipsRegisters::hilo()) ^= sign_extend((std::move(operand0) * std::move(operand1)), 64)
                ];
                break;
            }
            /* FIXME: hi/lo */
            case MIPS_INS_MULTU: {
                auto operand0 = operand(0);
                auto operand1 = operand(1);
                _[
                    regizter(MipsRegisters::hilo()) ^= zero_extend((std::move(operand0) * std::move(operand1)), 64)
                ];
                break;
            }

            case MIPS_INS_BEQL: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump(operand(0) == operand(op_count - 2),
                         (delayslot(taken)[jump(operand(op_count - 1))]).basicBlock(),
                         directSuccessorButOne())
                ];
                break;
            }
            case MIPS_INS_BEQ: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump(operand(0) == operand(op_count - 2),
                         (delayslot(taken)[jump(operand(op_count - 1))]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_BNEL: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump(~(operand(0) == operand(op_count - 2)),
                         (delayslot(taken)[jump(operand(op_count - 1))]).basicBlock(),
                         directSuccessorButOne())
                ];
                break;
            }
            case MIPS_INS_BNE: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump(~(operand(0) == operand(op_count - 2)),
                         (delayslot(taken)[jump(operand(op_count - 1))]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_BGEZL: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump((signed_(operand(0)) >= constant(0)),
                         (delayslot(taken)[jump(operand(1))]).basicBlock(),
                         directSuccessorButOne())
                ];
                break;
            }
            case MIPS_INS_BGEZ: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump((signed_(operand(0)) >= constant(0)),
                         (delayslot(taken)[jump(operand(1))]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_BGEZALL: {
                /* This is a conditional call */
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    regizter(MipsRegisters::ra()) ^= constant(nextDirectSuccessorAddress),
                    jump((signed_(operand(0)) >= constant(0)),
                         (delayslot(taken)[call(operand(1)), jump(directSuccessorButOne())]).basicBlock(),
                         directSuccessorButOne())
                ];
                break;
            }
            case MIPS_INS_BGEZAL: {
                /* This is a conditional call */
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    regizter(MipsRegisters::ra()) ^= constant(nextDirectSuccessorAddress),
                    jump((signed_(operand(0)) >= constant(0)),
                         (delayslot(taken)[call(operand(1)), jump(directSuccessorButOne())]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_BGTZL: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump((signed_(operand(0)) > constant(0)),
                         (delayslot(taken)[jump(operand(1))]).basicBlock(),
                         directSuccessorButOne())
                ];
                break;
            }
            case MIPS_INS_BGTZ: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump((signed_(operand(0)) > constant(0)),
                         (delayslot(taken)[jump(operand(1))]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_BLTZL: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump((signed_(operand(0)) < constant(0)),
                         (delayslot(taken)[jump(operand(1))]).basicBlock(),
                         directSuccessorButOne())
                ];
            }
            case MIPS_INS_BLTZ: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump((signed_(operand(0)) < constant(0)),
                         (delayslot(taken)[jump(operand(1))]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_BLTZALL: {
                /* This is a conditional call */
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    regizter(MipsRegisters::ra()) ^= constant(nextDirectSuccessorAddress),
                    jump((signed_(operand(0)) < constant(0)),
                         (delayslot(taken)[call(operand(1)), jump(directSuccessorButOne())]).basicBlock(),
                         directSuccessorButOne())
                ];
                break;
            }
            case MIPS_INS_BLTZAL: {
                /* This is a conditional call */
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    regizter(MipsRegisters::ra()) ^= constant(nextDirectSuccessorAddress),
                    jump((signed_(operand(0)) < constant(0)),
                         (delayslot(taken)[call(operand(1)), jump(directSuccessorButOne())]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_BLEZL: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump((signed_(operand(0)) <= constant(0)),
                         (delayslot(taken)[jump(operand(1))]).basicBlock(),
                         directSuccessorButOne())
                ];
                break;
            }
            case MIPS_INS_BLEZ: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump((signed_(operand(0)) <= constant(0)),
                         (delayslot(taken)[jump(operand(1))]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_BEQZ: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump((operand(0) == constant(0)),
                         (delayslot(taken)[jump(operand(1))]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_BNEZ: {
                MipsExpressionFactoryCallback taken(factory, program->createBasicBlock(), instruction);
                _[
                    jump(~(operand(0) == constant(0)),
                         (delayslot(taken)[jump(operand(1))]).basicBlock(),
                         directSuccessor())
                ];
                break;
            }
            case MIPS_INS_JR: {
                delayslot(_)[jump(operand(0))];
                break;
            }
            case MIPS_INS_JALR: {
                _[operand(0) ^= constant(nextDirectSuccessorAddress)];
                delayslot(_)[call(operand(op_count - 1))];
                break;
            }
            case MIPS_INS_BAL: /* Fall-through */
            case MIPS_INS_JAL: {
            	if (op_count == 1){
                	_[regizter(MipsRegisters::ra()) ^= constant(nextDirectSuccessorAddress)];
                	delayslot(_)[call(operand(0))];
            	} else {
                	_[operand(0) ^= constant(nextDirectSuccessorAddress)];
                  	delayslot(_)[call(operand(op_count - 1))];
            	}
                break;
            }
            case MIPS_INS_J: /* Fall-through */
            case MIPS_INS_B: {
                delayslot(_)[jump(operand(0))];
                break;
            }
            default: {
                _(std::make_unique<core::ir::InlineAssembly>());
                break;
            }
        } /* switch */
    }

    void createStatements(const MipsInstruction *instruction, core::ir::Program *program) {
        assert(instruction != nullptr);
        assert(program != nullptr);

        program_ = program;
        instruction_ = instruction;

        MipsExpressionFactory factory(architecture_);
        MipsExpressionFactoryCallback _(factory, program->getBasicBlockForInstruction(instruction), instruction);

        createStatements(_, instruction, program);
    }

private:
    core::arch::CapstoneInstructionPtr disassemble(const MipsInstruction *instruction) {
        capstone_.setMode(instruction->csMode());
        return capstone_.disassemble(instruction->addr(), instruction->bytes(), instruction->size());
    }

    unsigned int getOperandRegister(std::size_t index) const {
        if (index >= detail_->op_count) {
            throw core::irgen::InvalidInstructionException(tr("There is no operand %1.").arg(index));
        }

        const auto &operand = detail_->operands[index];

        if (operand.type == MIPS_OP_REG) {
            return operand.reg;
        } else {
            return MIPS_REG_INVALID;
        }
    }
    
    mips_op_type getOperandType(std::size_t index) const {
        if (index >= detail_->op_count) {
            throw core::irgen::InvalidInstructionException(tr("There is no operand %1.").arg(index));
        }

        const auto &operand = detail_->operands[index];
        return operand.type;
    }

    core::irgen::expressions::TermExpression operand(std::size_t index, SmallBitSize sizeHint = 32) const {
        return core::irgen::expressions::TermExpression(createTermForOperand(index, sizeHint));
    }

    std::unique_ptr<core::ir::Term> createTermForOperand(std::size_t index, SmallBitSize sizeHint) const {
        assert(index < boost::size(detail_->operands));

        const auto &operand = detail_->operands[index];
        
        switch (operand.type) {
            case MIPS_OP_INVALID:
                throw core::irgen::InvalidInstructionException(tr("The instruction does not have an argument with index %1").arg(index));
            case MIPS_OP_REG: {
                auto reg = getRegister(operand.reg)->memoryLocation();
                /* Transform a register $zero into a constant 0. */
                if (reg.domain() == core::ir::MemoryDomain::FIRST_REGISTER && !reg.addr()) {
                    return std::make_unique<core::ir::Constant>(SizedValue(sizeHint, 0LL));
                }
                return std::make_unique<core::ir::MemoryLocationAccess>(getRegister(operand.reg)->memoryLocation().resized(sizeHint));
            }
            case MIPS_OP_IMM: {
                /* Immediate value. */
                return std::make_unique<core::ir::Constant>(SizedValue(sizeHint, operand.imm));
            }
            case MIPS_OP_MEM: {
                return std::make_unique<core::ir::Dereference>(createDereferenceAddress(operand), core::ir::MemoryDomain::MEMORY, sizeHint);
            }
            default:
                unreachable();
        }
    }


  std::unique_ptr<core::ir::Dereference> createDereference(const cs_mips_op &operand) const {
        return std::make_unique<core::ir::Dereference>(
            createDereferenceAddress(operand), core::ir::MemoryDomain::MEMORY, 32);
    }

    std::unique_ptr<core::ir::Term> createDereferenceAddress(const cs_mips_op &operand) const {
        if (operand.type != MIPS_OP_MEM) {
            throw core::irgen::InvalidInstructionException(tr("Expected the operand to be a memory operand"));
        }

        const auto &mem = operand.mem;

        auto result = createRegisterAccess(mem.base);

        if (mem.disp != 0) {
            result = std::make_unique<core::ir::BinaryOperator>(
                core::ir::BinaryOperator::ADD,
                std::move(result),
                std::make_unique<core::ir::Constant>(SizedValue(result->size(), mem.disp)),
                result->size()
            );
        }

        return result;
    }

    static std::unique_ptr<core::ir::Term> createRegisterAccess(int reg) {
        return MipsInstructionAnalyzer::createTerm(getRegister(reg));
    }

    static const core::arch::Register *getRegister(int reg) {
        switch (reg) {
        #define REG(uppercase, lowercase) \
            case MIPS_REG_##uppercase: return MipsRegisters::lowercase();
            REG(ZERO,	zero)
            REG(AT,		at)
            REG(V0,		v0)
            REG(V1,		v1)
            REG(A0,		a0)
            REG(A1,		a1)
            REG(A2,		a2)
            REG(A3,		a3)
            REG(T0,		t0)
            REG(T1,		t1)
            REG(T2,		t2)
            REG(T3,		t3)
            REG(T4,		t4)
            REG(T5,		t5)
            REG(T6,		t6)
            REG(T7,		t7)
            REG(S0,		s0)
            REG(S1,		s1)
            REG(S2,		s2)
            REG(S3,		s3)
            REG(S4,		s4)
            REG(S5,		s5)
            REG(S6,		s6)
            REG(S7,		s7)
            REG(T8,		t8)
            REG(T9,		t9)
            REG(K0,		k0)
            REG(K1,		k1)
            REG(GP,		gp)
            REG(SP,		sp)
            REG(FP,		fp)
            /*REG(S8,	  s8)*/
            REG(RA,		ra)
            
            REG(F0,		f0)
            REG(F1,		f1)
            REG(F2,		f2)
            REG(F3,		f3)
            REG(F4,		f4)
            REG(F5,		f5)
            REG(F6,		f6)
            REG(F7,		f7)
            REG(F8,		f8)
            REG(F9,		f9)
            REG(F10,	f10)
            REG(F11,	f11)
            REG(F12,	f12)
            /*REG(FA0,	fa0)*/
            REG(F13,	f13)
            REG(F14,	f14)
            /*REG(FA1,	fa1)*/
            REG(F15,	f15)
            REG(F16,	f16)
            REG(F17,	f17)
            REG(F18,	f18)
            REG(F19,	f19)
            REG(F20,	f20)
            REG(F21,	f21)
            REG(F22,	f22)
            REG(F23,	f23)
            REG(F24,	f24)
            REG(F25,	f25)
            REG(F26,	f26)
            REG(F27,	f27)
            REG(F28,	f28)
            REG(F29,	f29)
            REG(F30,	f30)
            REG(F31,	f31)

            REG(HI,		hi)
            REG(LO,		lo)
        #undef REG

        default:
            throw core::irgen::InvalidInstructionException(tr("Invalid register number: %1").arg(reg));
        }
    }
};


MipsInstructionAnalyzer::MipsInstructionAnalyzer(const MipsArchitecture *architecture):
    impl_(std::make_unique<MipsInstructionAnalyzerImpl>(architecture))
{
}

MipsInstructionAnalyzer::~MipsInstructionAnalyzer() {}

void MipsInstructionAnalyzer::doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) {
    impl_->setInstructions(instructions());
    impl_->createStatements(checked_cast<const MipsInstruction *>(instruction), program);
}

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */
