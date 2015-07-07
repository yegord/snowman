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


NC_DEFINE_REGISTER_EXPRESSION(MipsRegisters, sp)
NC_DEFINE_REGISTER_EXPRESSION(MipsRegisters, gp)
NC_DEFINE_REGISTER_EXPRESSION(MipsRegisters, hilo)

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
        architecture_(architecture), capstone_(CS_ARCH_MIPS, CS_MODE_MIPS32), factory_(architecture)
    {
        assert(architecture_ != nullptr);
    }


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

  		/*
		 * The $zero-register always holds the value of zero (0).
         */
        _[
                    regizter(MipsRegisters::zero()) ^= constant(0)
        ];

        /* Describing semantics */
        switch (instr_->id) {
            case MIPS_INS_CACHE: /* Fall-through */
        	case MIPS_INS_BREAK:
        	case MIPS_INS_SYNC:
        	case MIPS_INS_SSNOP:
  	      	case MIPS_INS_NOP: {
    	    	break;
        	}
        	case MIPS_INS_ADD: /* Fall-through */       
			case MIPS_INS_ADDI:
        	case MIPS_INS_ADDIU:
        	case MIPS_INS_ADDU: {
				_[
					operand(0) ^= (operand(1) + signed_(operand(2)))
				];
    	    	break;
       	 	}
        	case MIPS_INS_AND: {
				_[
					operand(0) ^= (operand(1) & operand(2))
				];
    	    	break;
       	 	}
        	case MIPS_INS_ANDI: {
				_[
					operand(0) ^= (operand(1) & unsigned_(operand(2)))
				];
    	    	break;
       	 	}
        	case MIPS_INS_BEQL: /* Fall-through */    
         	case MIPS_INS_BEQ: {
        		_[
        			directSuccessor(),
        			jump((operand(0) == operand(1)), operand(2), directSuccessor())
        		];
    	    	break;
       	 	}
         	case MIPS_INS_BGEZ: {
        		_[
        			 directSuccessor(),
        			jump((unsigned_(operand(0)) >= constant(0)), operand(1), directSuccessor())
        		];
        	   	break;
       	 	}
#if 0 /* Fix me */
        	case MIPS_INS_BGEZAL: /* Fall-through */
         	case MIPS_INS_BGEZALL: {
        		/* This is a conditional call */
        		auto operand0 = unsigned_(operand(0));
        		_[regizter(MipsRegisters::ra()) ^= constant(instruction->endAddr())];
               if (operand0 >= constant(0))
                    _[call(operand(1))];
        	   	break;
       	 	}
#endif
         	case MIPS_INS_BGTZL: /* Fall-through */    
         	case MIPS_INS_BGTZ: {
        		_[
        			directSuccessor(),
        			jump((unsigned_(operand(0)) > constant(0)), operand(1), directSuccessor())
        		];
    	    	break;
       	 	}
        	case MIPS_INS_BLTZL: /* Fall-through */    
         	case MIPS_INS_BLTZ: {
        		_[
        			directSuccessor(),
        			jump((signed_(operand(0)) < constant(0)), operand(1), directSuccessor())
        		];
    	    	break;
       	 	}
#if 0 /* Fix me */
            case MIPS_INS_BLTZALL: /* Fall-through */  
            case MIPS_INS_BLTZAL: {
	        	/* This is a conditional call */
	      		auto operand0 = signed_(operand(0));
	      		auto zeroconst = constant(0);
        		_[regizter(MipsRegisters::ra()) ^= constant(instruction->endAddr())];
                auto result = std::make_unique<core::ir::BinaryOperator>(
                    core::ir::BinaryOperator::SIGNED_LESS,
                    std::move(operand0),
                    std::move(zeroconst),
                    operand0.size());
                
                if (result)
					 _[call(operand(1))];
    	    	break;
       	 	}
#endif
      	 	case MIPS_INS_BLEZL: /* Fall-through */    
         	case MIPS_INS_BLEZ: {
        		_[
        			directSuccessor(),
        			jump((signed_(operand(0)) <= constant(0)), operand(1), directSuccessor())
        		];
    	    	break;
       	 	}
         	case MIPS_INS_BNE: {
        		_[
        			 directSuccessor(),
        			jump(~(operand(0) == operand(1)), operand(2), directSuccessor())
        		];
    	    	break;
       	 	}
         	case MIPS_INS_BEQZ: {
        		_[
        			directSuccessor(),
        			jump((operand(0) == constant(0)), operand(1), directSuccessor())
        		];
    	    	break;
       	 	}
        	case MIPS_INS_BNEZ: {
        		_[
        			directSuccessor(),
        			jump(~(operand(0) == constant(0)), operand(1), directSuccessor())
        		];
    	    	break;
       	 	}
        	case MIPS_INS_DIVU: {
                auto operand2 = operand(2);
				_[
					regizter(MipsRegisters::hi()) ^= unsigned_(operand(1)) % unsigned_(std::move(operand2)),
					regizter(MipsRegisters::lo()) ^= unsigned_(operand(1)) / unsigned_(operand(2)),
					operand(0) ^= regizter(MipsRegisters::lo())
				];
    	    	break;
       	 	}
 			case MIPS_INS_LB: {
                _[
                    operand(0) ^= sign_extend(operand(1) & constant(0xff), 24)
                ];
    	    	break;
        	}
 			case MIPS_INS_LBU: {
                _[
                    operand(0) ^= zero_extend(operand(1) & constant(0xff), 24)
                ];
    	    	break;
        	}
 			case MIPS_INS_LUI: {
                _[
                    operand(0) ^=  (operand(1) << constant(16))
                ];
    	    	break;
        	}
        	case MIPS_INS_LWL: /* Fall-through */
        	case MIPS_INS_LWR:
      		case MIPS_INS_LW: {
                auto operand0 = operand(0);
			    auto operand1 = MemoryLocationExpression(core::ir::MemoryLocation(core::ir::MemoryDomain::MEMORY, 0, 32));
	            _[
	            	operand1 ^= operand(1),
					std::move(operand0) ^= std::move(operand1)
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
         	case MIPS_INS_MULTU: {
         		auto operand0 = operand(0);
                auto operand1 = operand(1);
				_[
					regizter(MipsRegisters::hilo()) ^= unsigned_(std::move(operand0)) * unsigned_(std::move(operand1))
				];
    	    	break;
       	 	}
 			case MIPS_INS_MOVE: {
                _[
                    operand(0) ^= operand(1)
                ];
    	    	break;
        	}
          	case MIPS_INS_NEG: /* Fall-through */ 	
 			case MIPS_INS_NEGU: {
                _[
                    operand(0) ^= (constant(0) - operand(1))
                ];
    	    	break;
        	}
 			case MIPS_INS_NOR: {
                _[
					operand(0) ^= ~operand(1)
                ];
    	    	break;
        	}
 			case MIPS_INS_NOT: {
                _[
					operand(0) ^= ~(operand(1) | operand(2))
                ];
    	    	break;
        	}
        	case MIPS_INS_OR: {
				_[
					operand(0) ^= (operand(1) | operand(2))
				];
    	    	break;
       	 	}
        	case MIPS_INS_ORI: {
				_[
					operand(0) ^= (operand(1) | unsigned_(operand(2)))
				];
    	    	break;
       	 	}
         	case MIPS_INS_SUB: /* Fall-through */       	 	
        	case MIPS_INS_SUBU: {
				_[
					operand(0) ^= (operand(1) - operand(2))
				];
    	    	break;
       	 	}
			case MIPS_INS_SLL: {
	            _[
	        		operand(0) ^= (operand(1) << operand(2))
	        	];
    	    	break;
        	}
			case MIPS_INS_SLLV: {
	            _[
	        		operand(0) ^= (operand(1) << (unsigned_(operand(2)) %  unsigned_(constant(32))))
	        	];
    	    	break;
        	}
         	case MIPS_INS_SLT: /* Fall-through */     
			case MIPS_INS_SLTI: {
                auto operand0 = signed_(operand(0));
	            _[
	        		operand0 ^= (signed_(operand(1)) < signed_(zero_extend(operand(2))))
	        	];
    	    	break;
        	}
         	case MIPS_INS_SLTU: /* Fall-through */     
			case MIPS_INS_SLTIU: {
                auto operand0 = unsigned_(operand(0));
	            _[
	        		operand0 ^= (unsigned_(operand(1)) < unsigned_(zero_extend(operand(2))))
	        	];
    	    	break;
        	}
        	case MIPS_INS_SWL: /* Fall-through */
         	case MIPS_INS_SWR:
      		case MIPS_INS_SW: {
                auto operand0 = operand(0);
			    auto operand1 = operand(1);

	            _[
					std::move(operand1) ^= std::move(operand0)
				];

    	    	break;
        	}
         	case MIPS_INS_XOR: /* Fall-through */       	 	
        	case MIPS_INS_XORI: {
				_[
					operand(0) ^= (operand(1) ^ operand(2))
				];

    	    	break;
       	 	}
			case MIPS_INS_JR: {
            	_[
            		directSuccessor(),
            		jump(operand(0))
            	];
            	break;
        	}
        	case MIPS_INS_JALR: /* Fall-through */
			case MIPS_INS_BAL:
			case MIPS_INS_JAL: {
	            	_[
	            		regizter(MipsRegisters::ra()) ^= constant(instruction->endAddr()),
                   	 	directSuccessor(),
                   	 	call(operand(0))
            		];
            	break;
        	}
        	case MIPS_INS_J: /* Fall-through */
        	case MIPS_INS_B: {
            		_[
            			directSuccessor(),
            			jump(operand(0))
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
        return core::irgen::expressions::TermExpression(createTermForOperand(index, sizeHint));
    }

    std::unique_ptr<core::ir::Term> createTermForOperand(std::size_t index, SmallBitSize sizeHint) const {
    	assert(index < boost::size(detail_->operands));

        const auto &operand = detail_->operands[index];
        
        switch (operand.type) {
            case MIPS_OP_INVALID:
                throw core::irgen::InvalidInstructionException(tr("The instruction does not have an argument with index %1").arg(index));
            case MIPS_OP_REG: {
            	return std::make_unique<core::ir::MemoryLocationAccess>(getRegister(operand.reg)->memoryLocation().resized(sizeHint));
            }
            case MIPS_OP_IMM: {
                /* Signed number, sign-extended to match the size of the other operand. */
                return std::make_unique<core::ir::Constant>(SizedValue(sizeHint, operand.imm));
            }
            case MIPS_OP_MEM:
                return std::make_unique<core::ir::Dereference>(createDereferenceAddress(operand), core::ir::MemoryDomain::MEMORY, sizeHint);
            default:
                unreachable();
        }
    }


  std::unique_ptr<core::ir::Dereference> createDereference(const cs_mips_op &operand) const {
        return std::make_unique<core::ir::Dereference>(
            createDereferenceAddress(operand), core::ir::MemoryDomain::MEMORY, 32);
    }

	/* FIXME */
    std::unique_ptr<core::ir::Term> createDereferenceAddress(const cs_mips_op &operand) const {
        if (operand.type != MIPS_OP_MEM) {
            throw core::irgen::InvalidInstructionException(tr("Expected the operand to be a memory operand"));
        }

    	const auto &mem = operand.mem;

        auto result = createRegisterAccess(mem.base);

	  	result = std::make_unique<core::ir::BinaryOperator>(
                core::ir::BinaryOperator::ADD,
                std::move(result),
                createRegisterAccess(operand.reg),
                result->size());

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
