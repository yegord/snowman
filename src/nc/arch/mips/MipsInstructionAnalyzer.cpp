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
       	 	case MIPS_INS_ADDIU: {
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

    core::irgen::expressions::TermExpression operand(std::size_t index, SmallBitSize sizeHint = 32) const {
        assert(index < boost::size(detail_->operands));

        const auto &operand = detail_->operands[index];

        return core::irgen::expressions::TermExpression(createTermForOperand(operand, sizeHint));
    }

    static std::unique_ptr<core::ir::Term> createTermForOperand(const cs_mips_op &operand, SmallBitSize sizeHint) {
        switch (operand.type) {
            case MIPS_OP_REG:
                return std::make_unique<core::ir::MemoryLocationAccess>(getRegister(operand.reg)->memoryLocation().resized(sizeHint));
            case MIPS_OP_IMM:
                return std::make_unique<core::ir::Constant>(SizedValue(sizeHint, operand.imm));
            case MIPS_OP_MEM:
                return std::make_unique<core::ir::Dereference>(createDereferenceAddress(operand), core::ir::MemoryDomain::MEMORY, sizeHint);
            default:
                unreachable();
        }
    }

    static std::unique_ptr<core::ir::Term> createDereferenceAddress(const cs_mips_op &operand) {
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
        #define REG(lowercase, uppercase) \
            case MIPS_REG_##uppercase: return MipsRegisters::lowercase();
			REG(zero,	ZERO)
			REG(at,     AT)
			REG(v0,     V0)
			REG(v1,     V1)
			REG(a0,     A0)
			REG(a1,     A1)
			REG(a2,     A2)
			REG(a3,     A3)
			REG(t0,     T0)
			REG(t1,     T1)
			REG(t2,     T2)
			REG(t3,     T3)
			REG(t4,     T4)
			REG(t5,     T5)
			REG(t6,     T6)
			REG(t7,     T7)
			REG(s0,     S0)
			REG(s1,     S1)
			REG(s2,     S2)
			REG(s3,     S3)
			REG(s4,     S4)
			REG(s5,     S5)
			REG(s6,     S6)
			REG(s7,     S7)
			REG(t8,     T8)
			REG(t9,     T9)
			REG(k0,     K0)
			REG(k1,     K1)
			REG(gp,     GP)
			REG(sp,     SP)
			REG(fp,     FP)
			/*REG(s8,     S8)*/
			REG(ra,     RA)
			
			REG(f0,		F0)
			REG(f1,		F1)
			REG(f2,		F2)
			REG(f3,		F3)
			REG(f4,		F4)
			REG(f5,		F5)
			REG(f6,		F6)
			REG(f7,		F7)
			REG(f8,		F8)
			REG(f9,		F9)
			REG(f10,	F10)
			REG(f11,	F11)
			REG(f12,	F12)
			/*REG(fa0,	FA0)*/
			REG(f13,	F13)
			REG(f14,	F14)
			/*REG(fa1,	FA1)*/
			REG(f15,	F15)
			REG(f16,	F16)
			REG(f17,	F17)
			REG(f18,	F18)
			REG(f19,	F19)
			REG(f20,	F20)
			REG(f21,	F21)
			REG(f22,	F22)
			REG(f23,	F23)
			REG(f24,	F24)
			REG(f25,	F25)
			REG(f26,	F26)
			REG(f27,	F27)
			REG(f28,	F28)
			REG(f29,	F29)
			REG(f30,	F30)
			REG(f31,	F31)

			REG(hi,     HI)
			REG(lo,     LO)
        #undef REG

        default:
            throw core::irgen::InvalidInstructionException(tr("Invalid register number: %1").arg(reg));
        }
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
