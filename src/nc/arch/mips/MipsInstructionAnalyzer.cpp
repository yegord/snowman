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

NC_DEFINE_REGISTER_EXPRESSION(MipsRegisters, zero)
NC_DEFINE_REGISTER_EXPRESSION(MipsRegisters, sp)

} // anonymous namespace

class MipsInstructionAnalyzerImpl {
    Q_DECLARE_TR_FUNCTIONS(MipsInstructionAnalyzerImpl)

	const MipsArchitecture *architecture_;
    core::arch::Capstone capstone_;
    MipsExpressionFactory factory_;
    core::ir::Program *program_;
    const MipsInstruction *instruction_;
    core::arch::CapstoneInstructionPtr instr_;
    const cs_mips *detail_;

public:
    MipsInstructionAnalyzerImpl(const MipsArchitecture *architecture):
        capstone_(CS_ARCH_MIPS, CS_MODE_MIPS32), factory_(architecture)
    {}

    void createStatements(const MipsInstruction *instruction, core::ir::Program *program) {
        assert(instruction != nullptr);
        assert(program != nullptr);

        program_ = program;
        instruction_ = instruction;

        instr_ = disassemble(instruction);
        /*assert(instr_ != nullptr);*/
        if(instr_ == nullptr)
        	return;
        detail_ = &instr_->detail->mips;

                
        core::ir::BasicBlock *cachedDirectSuccessor = nullptr;
        auto directSuccessor = [&]() -> core::ir::BasicBlock * {
            if (!cachedDirectSuccessor) {
                cachedDirectSuccessor = program->createBasicBlock(instruction->endAddr());
            }
            return cachedDirectSuccessor;
        };

  
        MipsExpressionFactory factory(architecture_);
        MipsExpressionFactoryCallback _(factory, program->getBasicBlockForInstruction(instruction), instruction);

        using namespace core::irgen::expressions;

        /* Describing semantics */
        switch (instr_->id) {
  	      case MIPS_INS_NOP: {
    	    	break;
        	}
       	 	case MIPS_INS_ADDU: {
				_[
					zero ^= constant(0),
					operand(0) ^= operand(1) + operand(2)
				];
    	    	break;
       	 	}
      		case MIPS_INS_MOVE: {
				_[
					zero ^= constant(0),
					operand(0) ^= operand(1)
				];
    	    	break;
        	}
       	 	default: {
        	    _(std::make_unique<core::ir::InlineAssembly>());
            	break;
 	       }
        } /* switch */
    }

private:
    core::arch::CapstoneInstructionPtr disassemble(const MipsInstruction *instruction) {
        capstone_.setMode(instruction->csMode());
        return capstone_.disassemble(instruction->addr(), instruction->bytes(), instruction->size());
    }

   core::irgen::expressions::TermExpression operand(std::size_t index) const {
        return core::irgen::expressions::TermExpression(createTermForOperand(index));
    }

    bool operandsAreTheSame(std::size_t index1, std::size_t index2) const {
        const auto &op1 = detail_->operands[index1];
        const auto &op2 = detail_->operands[index2];

        return op1.type == MIPS_OP_REG && op2.type == MIPS_OP_REG && op1.reg == op2.reg;
    }

    std::unique_ptr<core::ir::Term> createTermForOperand(std::size_t index) const {
        const auto &operand = detail_->operands[index];

        switch (operand.type) {
            case MIPS_OP_INVALID:
                throw core::irgen::InvalidInstructionException(tr("The instruction does not have an argument with index %1").arg(index));
            case MIPS_OP_REG: {
                auto result = createRegisterAccess(operand.reg);
                assert(result != nullptr);
                return result;
            }
            case MIPS_OP_IMM: {
                /* Signed number, sign-extended to match the size of the other operand. */
                return std::make_unique<core::ir::Constant>(SizedValue(4 * CHAR_BIT, operand.imm));
            }
            case MIPS_OP_MEM:
                return createDereference(operand);
            default:
                unreachable();
        }
    }

    std::unique_ptr<core::ir::Term> createRegisterAccess(unsigned reg) const {
        switch (reg) {
        case MIPS_REG_INVALID: return nullptr;
        #define REG(cs_name, nc_name) case MIPS_REG_##cs_name: return MipsInstructionAnalyzer::createTerm(MipsRegisters::nc_name());

			REG(ZERO,	zero)
			REG(AT,     at)
			REG(V0,     v0)
			REG(V1,     v1)
			REG(A0,     a0)
			REG(A1,     a1)
			REG(A2,     a2)
			REG(A3,     a3)
			REG(T0,     t0)
			REG(T1,     t1)
			REG(T2,     t2)
			REG(T3,     t3)
			REG(T4,     t4)
			REG(T5,     t5)
			REG(T6,     t6)
			REG(T7,     t7)
			REG(S0,     s0)
			REG(S1,     s1)
			REG(S2,     s2)
			REG(S3,     s3)
			REG(S4,     s4)
			REG(S5,     s5)
			REG(S6,     s6)
			REG(S7,     s7)
			REG(T8,     t8)
			REG(T9,     t9)
			REG(K0,     k0)
			REG(K1,     k1)
			REG(GP,     gp)
			REG(SP,     sp)
			REG(FP,     fp)
			/*REG(S8,     s8)*/
			REG(RA,     ra)
			
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

			REG(HI,     hi)
			REG(LO,     lo)
        #undef REG

        default:
            unreachable();
        }
    }

  std::unique_ptr<core::ir::Dereference> createDereference(const cs_mips_op &operand) const {
        return std::make_unique<core::ir::Dereference>(
            createDereferenceAddress(operand), core::ir::MemoryDomain::MEMORY, 4 * CHAR_BIT);
    }

    std::unique_ptr<core::ir::Term> createDereferenceAddress(const cs_mips_op &operand) const {
        if (operand.type != MIPS_OP_MEM) {
            throw core::irgen::InvalidInstructionException(tr("Expected the operand to be a memory operand"));
        }

        std::unique_ptr<core::ir::Term> result = createRegisterAccess(operand.mem.base);

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
       /* if (architecture_->bitness() == 16) {
            return detail_->prefix[2] == MIPS_PREFIX_OPSIZE ? 32 : 16;
        } else if (architecture_->bitness() == 32) {
            return detail_->prefix[2] == MIPS_PREFIX_OPSIZE ? 16 : 32;
        } else if (architecture_->bitness() == 64) {
            if (detail_->rex) {
                return 64;
            } else {
                return detail_->prefix[2] == MIPS_PREFIX_OPSIZE ? 16 : 32;
            }
        } else {
            unreachable();
        }*/
        return 32;
    }

    /**
     * \return Default operand size, inferred from the execution mode and instruction prefixes.
     */
    SmallBitSize addressSize() const {
        return 4 * CHAR_BIT;
    }
};


MipsInstructionAnalyzer::MipsInstructionAnalyzer(const MipsArchitecture *architecture):
    impl_(std::make_unique<MipsInstructionAnalyzerImpl>(architecture))
{}

MipsInstructionAnalyzer::~MipsInstructionAnalyzer() {}

void MipsInstructionAnalyzer::doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) {
    impl_->createStatements(checked_cast<const MipsInstruction *>(instruction), program);
}

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */
