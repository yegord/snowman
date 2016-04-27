/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "AllegrexInstructionAnalyzer.h"

#include <QCoreApplication>

#include <boost/range/size.hpp>

#include <nc/common/CheckedCast.h>
#include <nc/common/Foreach.h>
#include <nc/common/Unreachable.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/Program.h>
#include <nc/core/irgen/Expressions.h>
#include <nc/core/irgen/InvalidInstructionException.h>

#include <nc/core/arch/Instructions.h>

#include "AllegrexArchitecture.h"
#include "AllegrexInstruction.h"
#include "AllegrexRegisters.h"
#include "Allegrex.h"

namespace nc {
    namespace arch {
        namespace allegrex {

            namespace {

                class AllegrexExpressionFactory : public core::irgen::expressions::ExpressionFactory<AllegrexExpressionFactory> {
                public:
                    AllegrexExpressionFactory(const core::arch::Architecture *architecture) :
                        core::irgen::expressions::ExpressionFactory<AllegrexExpressionFactory>(architecture)
                    {}
                };

                typedef core::irgen::expressions::ExpressionFactoryCallback<AllegrexExpressionFactory> AllegrexExpressionFactoryCallback;

                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, zero);
                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, sp);
                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, gp);
                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, ra);
                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, hi);
                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, lo);

                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, tmp64);
                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, tmp32);

            } // anonymous namespace

            class AllegrexInstructionAnalyzerImpl {
                Q_DECLARE_TR_FUNCTIONS(AllegrexInstructionAnalyzerImpl);

                const AllegrexArchitecture *architecture_;
                AllegrexExpressionFactory factory_;
                core::ir::Program *program_;
                const AllegrexInstruction *instruction_;
                const core::arch::Instructions *instructions_;

            public:
                AllegrexInstructionAnalyzerImpl(const AllegrexArchitecture *architecture) :
                    architecture_(architecture), factory_(architecture)
                {
                    assert(architecture_ != nullptr);
                }

                void setInstructions(const core::arch::Instructions *instructions) {
                    instructions_ = instructions;
                }

                void createStatements(const AllegrexInstruction *instruction, core::ir::Program *program) {
                    assert(instruction != nullptr);
                    assert(program != nullptr);

                    program_ = program;
                    instruction_ = instruction;

                    AllegrexExpressionFactory factory(architecture_);

                    AllegrexExpressionFactoryCallback _(factory, program->getBasicBlockForInstruction(instruction), instruction);

                    createStatements(_, instruction, program, nullptr);
                }

            private:
                const core::arch::Register *gpr_(int index) const {
                    switch (index) {
                    case allegrex::R_GPR0 : return AllegrexRegisters::zero();
                    case allegrex::R_GPR1 : return AllegrexRegisters::at();
                    case allegrex::R_GPR2 : return AllegrexRegisters::v0();
                    case allegrex::R_GPR3 : return AllegrexRegisters::v1();
                    case allegrex::R_GPR4 : return AllegrexRegisters::a0();
                    case allegrex::R_GPR5 : return AllegrexRegisters::a1();
                    case allegrex::R_GPR6 : return AllegrexRegisters::a2();
                    case allegrex::R_GPR7 : return AllegrexRegisters::a3();
                    case allegrex::R_GPR8 : return AllegrexRegisters::t0();
                    case allegrex::R_GPR9 : return AllegrexRegisters::t1();
                    case allegrex::R_GPR10: return AllegrexRegisters::t2();
                    case allegrex::R_GPR11: return AllegrexRegisters::t3();
                    case allegrex::R_GPR12: return AllegrexRegisters::t4();
                    case allegrex::R_GPR13: return AllegrexRegisters::t5();
                    case allegrex::R_GPR14: return AllegrexRegisters::t6();
                    case allegrex::R_GPR15: return AllegrexRegisters::t7();
                    case allegrex::R_GPR16: return AllegrexRegisters::s0();
                    case allegrex::R_GPR17: return AllegrexRegisters::s1();
                    case allegrex::R_GPR18: return AllegrexRegisters::s2();
                    case allegrex::R_GPR19: return AllegrexRegisters::s3();
                    case allegrex::R_GPR20: return AllegrexRegisters::s4();
                    case allegrex::R_GPR21: return AllegrexRegisters::s5();
                    case allegrex::R_GPR22: return AllegrexRegisters::s6();
                    case allegrex::R_GPR23: return AllegrexRegisters::s7();
                    case allegrex::R_GPR24: return AllegrexRegisters::t8();
                    case allegrex::R_GPR25: return AllegrexRegisters::t9();
                    case allegrex::R_GPR26: return AllegrexRegisters::k0();
                    case allegrex::R_GPR27: return AllegrexRegisters::k1();
                    case allegrex::R_GPR28: return AllegrexRegisters::gp();
                    case allegrex::R_GPR29: return AllegrexRegisters::sp();
                    case allegrex::R_GPR30: return AllegrexRegisters::fp();
                    case allegrex::R_GPR31: return AllegrexRegisters::ra();
                    default:
                        throw core::irgen::InvalidInstructionException(tr("Invalid GPR number: %1").arg(index));
                    }
                }

                const core::arch::Register *fpr_(int index) const {
                    switch (index) {
                    case allegrex::R_FPR0 : return AllegrexRegisters::f0();
                    case allegrex::R_FPR1 : return AllegrexRegisters::f1();
                    case allegrex::R_FPR2 : return AllegrexRegisters::f2();
                    case allegrex::R_FPR3 : return AllegrexRegisters::f3();
                    case allegrex::R_FPR4 : return AllegrexRegisters::f4();
                    case allegrex::R_FPR5 : return AllegrexRegisters::f5();
                    case allegrex::R_FPR6 : return AllegrexRegisters::f6();
                    case allegrex::R_FPR7 : return AllegrexRegisters::f7();
                    case allegrex::R_FPR8 : return AllegrexRegisters::f8();
                    case allegrex::R_FPR9 : return AllegrexRegisters::f9();
                    case allegrex::R_FPR10: return AllegrexRegisters::f10();
                    case allegrex::R_FPR11: return AllegrexRegisters::f11();
                    case allegrex::R_FPR12: return AllegrexRegisters::f12();
                    case allegrex::R_FPR13: return AllegrexRegisters::f13();
                    case allegrex::R_FPR14: return AllegrexRegisters::f14();
                    case allegrex::R_FPR15: return AllegrexRegisters::f15();
                    case allegrex::R_FPR16: return AllegrexRegisters::f16();
                    case allegrex::R_FPR17: return AllegrexRegisters::f17();
                    case allegrex::R_FPR18: return AllegrexRegisters::f18();
                    case allegrex::R_FPR19: return AllegrexRegisters::f19();
                    case allegrex::R_FPR20: return AllegrexRegisters::f20();
                    case allegrex::R_FPR21: return AllegrexRegisters::f21();
                    case allegrex::R_FPR22: return AllegrexRegisters::f22();
                    case allegrex::R_FPR23: return AllegrexRegisters::f23();
                    case allegrex::R_FPR24: return AllegrexRegisters::f24();
                    case allegrex::R_FPR25: return AllegrexRegisters::f25();
                    case allegrex::R_FPR26: return AllegrexRegisters::f26();
                    case allegrex::R_FPR27: return AllegrexRegisters::f27();
                    case allegrex::R_FPR28: return AllegrexRegisters::f28();
                    case allegrex::R_FPR29: return AllegrexRegisters::f29();
                    case allegrex::R_FPR30: return AllegrexRegisters::f30();
                    case allegrex::R_FPR31: return AllegrexRegisters::f31();
                    default:
                        throw core::irgen::InvalidInstructionException(tr("Invalid FPR number: %1").arg(index));
                    }
                }

                const AllegrexInstruction *getDelayslotInstruction(const AllegrexInstruction *instruction) {
                    auto delayslotInstruction = checked_cast<const AllegrexInstruction *>(instructions_->get(instruction->endAddr()).get());
                    if (!delayslotInstruction) {
                        throw core::irgen::InvalidInstructionException(tr("Cannot find a delay slot at 0x%1.").arg(instruction->endAddr(), 0, 16));
                    }
                    return delayslotInstruction;
                };

                core::ir::BasicBlock *createStatements(AllegrexExpressionFactoryCallback & _,
                                                       const AllegrexInstruction *instruction,
                                                       core::ir::Program *program,
                                                       const AllegrexInstruction *delayslotOwner) {

                    allegrex_operand operand[8];

                    auto insn = disassemble(instruction, operand);
                    assert(insn);

                    _[core::irgen::expressions::regizter(AllegrexRegisters::zero()) ^= core::irgen::expressions::constant(0)];

                    auto delayslotCallback = [&](AllegrexExpressionFactoryCallback &callback) -> AllegrexExpressionFactoryCallback & {
			if (auto delayslotInstruction = getDelayslotInstruction(instruction)) {
                            callback.setBasicBlock(createStatements(callback, delayslotInstruction, program, instruction));
                        }
                        return callback;
                    };

                    core::ir::BasicBlock *cachedDirectSuccessor = nullptr;
                    auto directSuccessor = [&]() -> core::ir::BasicBlock * {
			if (!cachedDirectSuccessor) {
                            cachedDirectSuccessor = program->createBasicBlock(instruction->endAddr());
                        }
                        return cachedDirectSuccessor;
                    };

                    core::ir::BasicBlock *cachedDirectSuccessorButOne = nullptr;
                    auto directSuccessorButOne = [&]() -> core::ir::BasicBlock * {
			if (!cachedDirectSuccessorButOne) {
                            cachedDirectSuccessorButOne = program->createBasicBlock(instruction->endAddr() + instruction->size());
                        }
                        return cachedDirectSuccessorButOne;
                    };

                    auto gpr = [&](int index) -> core::irgen::expressions::TermExpression {
                        if (index != R_ZERO) {
                            auto &reg = gpr_(operand[index].reg)->memoryLocation();
                            return core::irgen::expressions::TermExpression(std::make_unique<core::ir::MemoryLocationAccess>(reg.resized(32)));
                        }
                        return core::irgen::expressions::TermExpression(std::make_unique<core::ir::Constant>(SizedValue(32, 0)));
                    };

                    auto gpr16 = [&](int index) -> core::irgen::expressions::TermExpression {
                        if (index != R_ZERO) {
                            auto &reg = gpr_(operand[index].reg)->memoryLocation();
                            return core::irgen::expressions::TermExpression(std::make_unique<core::ir::MemoryLocationAccess>(reg.resized(16)));
                        }
                        return core::irgen::expressions::TermExpression(std::make_unique<core::ir::Constant>(SizedValue(16, 0)));
                    };

                    auto gpr8 = [&](int index) -> core::irgen::expressions::TermExpression {
                        if (index != R_ZERO) {
                            auto &reg = gpr_(operand[index].reg)->memoryLocation();
                            return core::irgen::expressions::TermExpression(std::make_unique<core::ir::MemoryLocationAccess>(reg.resized(8)));
                        }
                        return core::irgen::expressions::TermExpression(std::make_unique<core::ir::Constant>(SizedValue(8, 0)));
                    };

                    auto gpr5 = [&](int index) -> core::irgen::expressions::TermExpression {
                        if (index != R_ZERO) {
                            auto &reg = gpr_(operand[index].reg)->memoryLocation();
                            return core::irgen::expressions::TermExpression(std::make_unique<core::ir::MemoryLocationAccess>(reg.resized(5)));
                        }
                        return core::irgen::expressions::TermExpression(std::make_unique<core::ir::Constant>(SizedValue(5, 0)));
                    };

                    auto fpr = [&](int index) -> core::irgen::expressions::TermExpression {
                        auto &reg = fpr_(operand[index].reg)->memoryLocation();
                        return core::irgen::expressions::TermExpression(std::make_unique<core::ir::MemoryLocationAccess>(reg.resized(32)));
                    };

                    auto vpr = [&](int index) -> core::irgen::expressions::TermExpression {
                        throw core::irgen::InvalidInstructionException(tr("Invalid VPR number: %1").arg(index));
                    };

                    auto imm = [&](int index) -> core::irgen::expressions::TermExpression {
                        return core::irgen::expressions::TermExpression(std::make_unique<core::ir::Constant>(SizedValue(32, operand[index].imm)));
                    };

                    auto mem = [&](int index, int sizeHint) -> core::irgen::expressions::TermExpression {
                        if (operand[index].mem.disp) {
                            return core::irgen::expressions::TermExpression(
                                std::make_unique<core::ir::Dereference>(
                                    std::make_unique<core::ir::BinaryOperator>(
                                        core::ir::BinaryOperator::ADD,
                                        AllegrexInstructionAnalyzer::createTerm(gpr_(operand[index].mem.base)),
                                        std::make_unique<core::ir::Constant>(SizedValue(32, operand[index].mem.disp)),
                                        32
                                    ),
                                    core::ir::MemoryDomain::MEMORY,
                                    sizeHint
                                )
                            );
                        } else {
                            return core::irgen::expressions::TermExpression(
                                std::make_unique<core::ir::Dereference>(
                                    AllegrexInstructionAnalyzer::createTerm(gpr_(operand[index].mem.base)),
                                    core::ir::MemoryDomain::MEMORY,
                                    sizeHint
                                )
                            );
                        }
                    };
	
					using namespace core::irgen::expressions;

                    switch (insn->id)
                    {
                    case I_ADD:
                    case I_ADDU: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= gpr(1) + gpr(2)];
                        break;
                    }
                    case I_ADDI:
                    case I_ADDIU: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= gpr(1) + imm(2)];
                        break;
                    }
                    case I_AND: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= gpr(1) & gpr(2)];
                        break;
                    }
                    case I_ANDI: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= gpr(1) & imm(2)];
                        break;
                    }
                    case I_BEQ:  {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(2))
                        ];
                        _[
                            jump(gpr(0) == gpr(1),
                                 taken.basicBlock(),
                                 directSuccessor())
                        ];
                        break;
                    }
                    case I_BEQL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(2))
                        ];
                        _[
                            jump(gpr(0) == gpr(1),
                                 taken.basicBlock(),
                                 directSuccessorButOne())
                        ];
                        break;
                    }
                    case I_BGEZ: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(1))
                        ];
                        _[
                            jump(signed_(gpr(0)) >= constant(0),
                                 taken.basicBlock(),
                                 directSuccessor())
                        ];
                        break;
                    }
                    case I_BGEZAL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            call(imm(1)), jump(directSuccessorButOne())
                        ];
                        _[
                            jump(signed_(gpr(0)) >= constant(0),
                                 taken.basicBlock(),
                                 directSuccessor())
                        ];
                        break;
                    }
                    case I_BGEZALL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            call(imm(1)), jump(directSuccessorButOne())
                        ];
                        _[
                            jump(signed_(gpr(0)) >= constant(0),
                                 taken.basicBlock(),
                                 directSuccessorButOne())
                        ];
                        break;
                    }
                    case I_BGEZL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(1))
                        ];
                        _[
                            jump(signed_(gpr(0)) >= constant(0),
                                 taken.basicBlock(),
                                 directSuccessorButOne())
                        ];
                        break;
                    }
                    case I_BGTZ: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(1))
                        ];
                        _[
                            jump(signed_(gpr(0)) > constant(0),
                                 taken.basicBlock(),
                                 directSuccessor())
                        ];
                        break;
                    }
                    case I_BGTZL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(1))
                        ];
                        _[
                            jump(signed_(gpr(0)) > constant(0),
                                 taken.basicBlock(),
                                 directSuccessorButOne())
                        ];
                        break;
                    }
                    case I_BITREV: {
                        auto rd = unsigned_(gpr(0));
                        auto rt = unsigned_(gpr(1));
                        auto swapB = [&](unsigned shift, unsigned mask) {
                            return ((std::move(rt) >> core::irgen::expressions::constant(shift)) & core::irgen::expressions::constant(mask)) | ((std::move(rt) & core::irgen::expressions::constant(mask)) << core::irgen::expressions::constant(shift));
                        };
                        auto swapS = [&](unsigned shift, unsigned mask) {
                            return ((std::move(rd) >> core::irgen::expressions::constant(shift)) & core::irgen::expressions::constant(mask)) | ((std::move(rd) & core::irgen::expressions::constant(mask)) << core::irgen::expressions::constant(shift));
                        };
                        auto swapE = [&](unsigned shift) {
                            return (std::move(rd) >> core::irgen::expressions::constant(shift)) | (std::move(rd) << core::irgen::expressions::constant(shift));
                        };
                        _[
                            std::move(rd) ^= swapB(1, 0x55555555),
                            std::move(rd) ^= swapS(2, 0x33333333),
                            std::move(rd) ^= swapS(4, 0x0F0F0F0F),
                            std::move(rd) ^= swapS(8, 0x00FF00FF),
                            std::move(rd) ^= swapE(16)
                        ];
                        break;
                    }
                    case I_BLEZ: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(1))
                        ];
                        _[
                            jump(signed_(gpr(0)) <= constant(0),
                                 taken.basicBlock(),
                                 directSuccessor())
                        ];
                        break;
                    }
                    case I_BLEZL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(1))
                        ];
                        _[
                            jump(signed_(gpr(0)) <= constant(0),
                                 taken.basicBlock(),
                                 directSuccessorButOne())
                        ];
                        break;
                    }
                    case I_BLTZ: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(1))
                        ];
                        _[
                            jump(signed_(gpr(0)) < constant(0),
                                 taken.basicBlock(),
                                 directSuccessor())
                        ];
                        break;
                    }
                    case I_BLTZL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(1))
                        ];
                        _[
                            jump(signed_(gpr(0)) < constant(0),
                                 taken.basicBlock(),
                                 directSuccessorButOne())
                        ];
                        break;
                    }
                    case I_BLTZAL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            call(imm(1)), jump(directSuccessorButOne())
                        ];
                        _[
                            jump(signed_(gpr(0)) < constant(0),
                                 taken.basicBlock(),
                                 directSuccessor())
                        ];
                        break;
                    }
                    case I_BLTZALL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            call(imm(1)), jump(directSuccessorButOne())
                        ];
                        _[
                            jump(signed_(gpr(0)) < constant(0),
                                 taken.basicBlock(),
                                 directSuccessorButOne())
                        ];
                        break;
                    }
                    case I_BNE: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(2))
                        ];
                        _[
                            jump(~(gpr(0) == gpr(1)),
                                 taken.basicBlock(),
                                 directSuccessor())
                        ];
                        break;
                    }
                    case I_BNEL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(2))
                        ];
                        _[
                            jump(~(gpr(0) == gpr(1)),
                                 taken.basicBlock(),
                                 directSuccessorButOne())
                        ];
                        break;
                    }
                    case I_BREAK:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_CACHE:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_CFC0:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_CLO:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_CLZ:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_CTC0:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_MAX:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_MIN:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_DBREAK:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_DIV: {
                        auto rs = signed_(sign_extend(gpr(0), 64));
                        auto rt = signed_(sign_extend(gpr(1), 64));
                        _[
                            lo ^= truncate((rs / rt), 32),
                            hi ^= truncate((rs % rt), 32)
                        ];
                        break;
                    }
                    case I_DIVU: {
                        auto rs = unsigned_(zero_extend(gpr(0), 64));
                        auto rt = unsigned_(zero_extend(gpr(1), 64));
                        _[
                            lo ^= truncate((rs / rt), 32),
                            hi ^= truncate((rs % rt), 32)
                        ];
                        break;
                    }
                    case I_DRET:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_ERET:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_EXT:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_INS:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;                   
                    case I_J: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            jump(imm(0))
                        ];
                        _[jump(taken.basicBlock())];
                        break;
                    }
                    case I_JR: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block);
                        if (operand[0].reg == R_RA) {
                            taken[jump(return_address())];
                        } else {
                            taken[jump(gpr(0))];
                        }
                        _[jump(taken.basicBlock())];
                        break;
                    }
                    case I_JALR: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            call(gpr(0)), jump(directSuccessorButOne())
                        ];
                        _[jump(taken.basicBlock())];
                        break;
                    }
                    case I_JAL: {
                        auto block = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), instruction);
                        auto taken = delayslotCallback(block)[
                            call(imm(0)), jump(directSuccessorButOne())
                        ];
                        _[jump(taken.basicBlock())];
                        break;
                    }
                    case I_LB:
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= sign_extend(mem(1, 8))];
                        else
                            _[zero ^= sign_extend(mem(1, 8))];
                        break;
                    case I_LBU:
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= zero_extend(mem(1, 8))];
                        else
                            _[zero ^= zero_extend(mem(1, 8))];
                        break;
                    case I_LH:
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= sign_extend(mem(1, 16))];
                        else
                            _[zero ^= sign_extend(mem(1, 16))];
                        break;
                    case I_LHU:
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= zero_extend(mem(1, 16))];
                        else
                            _[zero ^= zero_extend(mem(1, 16))];
                        break;
                    case I_LL:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_LUI: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= (imm(1) << constant(16))];
                        break;
                    }
                    case I_LW: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= mem(1, 32)];
                        break;
                    }
                    case I_LWL:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_LWR:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_MADD: {
                        auto rs = sign_extend(gpr(0), 64);
                        auto rt = sign_extend(gpr(0), 64);
                        _[
                            tmp64 ^= sign_extend(hi, 64) << constant(32) + zero_extend(lo, 64) + std::move(rs) * std::move(rt),

                            lo ^= truncate(tmp64, 32),
                            hi ^= truncate(signed_(tmp64) >> constant(32), 32)
                        ];
                        break;
                    }
                    case I_MADDU: {
                        auto rs = zero_extend(gpr(0), 64);
                        auto rt = zero_extend(gpr(0), 64);
                        _[
                            tmp64 ^= zero_extend(hi, 64) << constant(32) + zero_extend(lo, 64) + std::move(rs) * std::move(rt),

                            lo ^= truncate(tmp64, 32),
                            hi ^= truncate(unsigned_(tmp64) >> constant(32), 32)
                        ];
                        break;
                    }
                    case I_MFC0:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_MFDR:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_MFHI: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= hi];
                        break;
                    }
                    case I_MFIC:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_MFLO: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= lo];
                        break;
                    }
                    case I_MOVN: {
                        auto move = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), delayslotOwner ? delayslotOwner : instruction)[
                            gpr(0) ^= gpr(1)
                        ];
                        _[
                            jump(gpr(2),
                                 move.basicBlock(),
                                 directSuccessor())
                        ];
                        return move.basicBlock();
                    }
                    case I_MOVZ:{
                        auto move = AllegrexExpressionFactoryCallback(factory_, program->createBasicBlock(), delayslotOwner ? delayslotOwner : instruction)[
                            gpr(0) ^= gpr(1)
                        ];
                        _[
                            jump(gpr(2),
                                 directSuccessor(),
                                 move.basicBlock())
                        ];
                        return move.basicBlock();
                    }
                    case I_MSUB: {
                        auto rs = sign_extend(gpr(0), 64);
                        auto rt = sign_extend(gpr(0), 64);
                        _[
                            tmp64 ^= sign_extend(hi, 64) << constant(32) + zero_extend(lo, 64) - std::move(rs) * std::move(rt),

                            lo ^= truncate(tmp64, 32),
                            hi ^= truncate(signed_(tmp64) >> constant(32), 32)
                        ];
                    }
                    case I_MSUBU: {
                        auto rs = zero_extend(gpr(0), 64);
                        auto rt = zero_extend(gpr(0), 64);
                        _[
                            tmp64 ^= zero_extend(hi, 64) << constant(32) + zero_extend(lo, 64) - std::move(rs) * std::move(rt),

                            lo ^= truncate(tmp64, 32),
                            hi ^= truncate(unsigned_(tmp64) >> constant(32), 32)
                        ];
                        break;
                    }
                    case I_MTC0:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_MTDR:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_MTIC:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_HALT:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_MTHI: {
                        _[hi ^= gpr(0)];
                        break;
                    }
                    case I_MTLO: {
                        _[lo ^= gpr(0)];
                        break;
                    }
                    case I_MULT: {
                        auto rs = sign_extend(gpr(0), 64);
                        auto rt = sign_extend(gpr(0), 64);
                        _[
                            tmp64 ^= std::move(rs) * std::move(rt),

                            lo ^= truncate(tmp64, 32),
                            hi ^= truncate(signed_(tmp64) >> constant(32), 32)
                        ];
                        break;
                    }
                    case I_MULTU: {
                        auto rs = zero_extend(gpr(0), 64);
                        auto rt = zero_extend(gpr(0), 64);
                        _[
                            tmp64 ^= std::move(rs) * std::move(rt),

                            lo ^= truncate(tmp64, 32),
                            hi ^= truncate(unsigned_(tmp64) >> constant(32), 32)
                        ];
                        break;
                    }
                    case I_NOR: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= ~(gpr(1) | gpr(2))];
                        break;
                    }
                    case I_OR: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= gpr(1) | gpr(2)];
                        break;
                    }
                    case I_ORI: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= gpr(1) | imm(2)];
                        break;
                    }
                    case I_ROTR:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_ROTV:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_SEB: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= sign_extend(gpr8(1))];
                        break;
                    }
                    case I_SEH: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= sign_extend(gpr16(1))];
                        break;
                    }
                    case I_SB: {
                        _[mem(1, 8) ^= truncate(gpr(0), 8)];
                        break;
                    }
                    case I_SC:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_SH: {
                        _[mem(1, 16) ^= truncate(gpr(0), 16)];
                        break;
                    }
                    case I_SLLV: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= (unsigned_(gpr(1)) << zero_extend(gpr5(2)))];
                        break;
                    }
                    case I_SLL: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= (unsigned_(gpr(1)) << imm(2))];
                        break;
                    }
                    case I_SLT: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= zero_extend(signed_(gpr(1)) < signed_(gpr(2)))];
                        break;
                    }
                    case I_SLTI: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= zero_extend(signed_(gpr(1)) < signed_(imm(2)))];
                        break;
                    }
                    case I_SLTIU: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= zero_extend(unsigned_(gpr(1)) < unsigned_(imm(2)))];
                        break;
                    }
                    case I_SLTU: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= zero_extend(unsigned_(gpr(1)) < unsigned_(gpr(2)))];
                        break;
                    }
                    case I_SRA: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= (signed_(gpr(1)) >> imm(2))];
                        break;
                    }
                    case I_SRAV: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= (signed_(gpr(1)) >> zero_extend(gpr5(2)))];
                        break;
                    }
                    case I_SRLV: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= (unsigned_(gpr(1)) >> zero_extend(gpr5(2)))];
                        break;
                    }
                    case I_SRL: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= (unsigned_(gpr(1)) >> imm(2))];
                        break;
                    }
                    case I_SW: {
                        _[mem(1, 32) ^= gpr(0)];
                        break;
                    }
                    case I_SWL: {
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    }
                    case I_SWR: {
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    }
                    case I_SUB:
                    case I_SUBU: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= gpr(1) - gpr(2)];
                        break;
                    }
                    case I_SYNC:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_SYSCALL:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_XOR: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= gpr(1) ^ gpr(2)];
                        break;
                    }
                    case I_XORI: {
                        if (operand[0].reg != R_ZERO)
                            _[gpr(0) ^= gpr(1) ^ imm(2)];
                        break;
                    }
                    case I_WSBH:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    case I_WSBW:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;

                    case I_ABS_S:
                    case I_ADD_S:
                    case I_BC1F:
                    case I_BC1FL:
                    case I_BC1T:
                    case I_BC1TL:
                    case I_C_F_S:
                    case I_C_UN_S:
                    case I_C_EQ_S:
                    case I_C_UEQ_S:
                    case I_C_OLT_S:
                    case I_C_ULT_S:
                    case I_C_OLE_S:
                    case I_C_ULE_S:
                    case I_C_SF_S:
                    case I_C_NGLE_S:
                    case I_C_SEQ_S:
                    case I_C_NGL_S:
                    case I_C_LT_S:
                    case I_C_NGE_S:
                    case I_C_LE_S:
                    case I_C_NGT_S:
                    case I_CEIL_W_S:
                    case I_CFC1:
                    case I_CTC1:
                    case I_CVT_S_W:
                    case I_CVT_W_S:
                    case I_DIV_S:
                    case I_FLOOR_W_S:
                    case I_LWC1:
                    case I_MFC1:
                    case I_MOV_S:
                    case I_MTC1:
                    case I_MUL_S:
                    case I_NEG_S:
                    case I_ROUND_W_S:
                    case I_SQRT_S:
                    case I_SUB_S:
                    case I_SWC1:
                    case I_TRUNC_W_S:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;

                    case I_BVF:
                    case I_BVFL:
                    case I_BVT:
                    case I_BVTL:
                    case I_LV_Q:
                    case I_LV_S:
                    case I_LVL_Q:
                    case I_LVR_Q:
                    case I_MFV:
                    case I_MFVC:
                    case I_MTV:
                    case I_MTVC:
                    case I_SV_Q:
                    case I_SV_S:
                    case I_SVL_Q:
                    case I_SVR_Q:
                    case I_VABS_P:
                    case I_VABS_Q:
                    case I_VABS_S:
                    case I_VABS_T:
                    case I_VADD_P:
                    case I_VADD_Q:
                    case I_VADD_S:
                    case I_VADD_T:
                    case I_VASIN_P:
                    case I_VASIN_Q:
                    case I_VASIN_S:
                    case I_VASIN_T:
                    case I_VAVG_P:
                    case I_VAVG_Q:
                    case I_VAVG_T:
                    case I_VBFY1_P:
                    case I_VBFY1_Q:
                    case I_VBFY2_Q:
                    case I_VCMOVF_P:
                    case I_VCMOVF_Q:
                    case I_VCMOVF_S:
                    case I_VCMOVF_T:
                    case I_VCMOVT_P:
                    case I_VCMOVT_Q:
                    case I_VCMOVT_S:
                    case I_VCMOVT_T:
                    case I_VCMP_P:
                    case I_VCMP_Q:
                    case I_VCMP_S:
                    case I_VCMP_T:
                    case I_VCOS_P:
                    case I_VCOS_Q:
                    case I_VCOS_S:
                    case I_VCOS_T:
                    case I_VCRS_T:
                    case I_VCRSP_T:
                    case I_VCST_P:
                    case I_VCST_Q:
                    case I_VCST_S:
                    case I_VCST_T:
                    case I_VDET_P:
                    case I_VDIV_P:
                    case I_VDIV_Q:
                    case I_VDIV_S:
                    case I_VDIV_T:
                    case I_VDOT_P:
                    case I_VDOT_Q:
                    case I_VDOT_T:
                    case I_VEXP2_P:
                    case I_VEXP2_Q:
                    case I_VEXP2_S:
                    case I_VEXP2_T:
                    case I_VF2H_P:
                    case I_VF2H_Q:
                    case I_VF2ID_P:
                    case I_VF2ID_Q:
                    case I_VF2ID_S:
                    case I_VF2ID_T:
                    case I_VF2IN_P:
                    case I_VF2IN_Q:
                    case I_VF2IN_S:
                    case I_VF2IN_T:
                    case I_VF2IU_P:
                    case I_VF2IU_Q:
                    case I_VF2IU_S:
                    case I_VF2IU_T:
                    case I_VF2IZ_P:
                    case I_VF2IZ_Q:
                    case I_VF2IZ_S:
                    case I_VF2IZ_T:
                    case I_VFAD_P:
                    case I_VFAD_Q:
                    case I_VFAD_T:
                    case I_VFIM_S:
                    case I_VFLUSH:
                    case I_VH2F_P:
                    case I_VH2F_S:
                    case I_VHDP_P:
                    case I_VHDP_Q:
                    case I_VHDP_T:
                    case I_VHTFM2_P:
                    case I_VHTFM3_T:
                    case I_VHTFM4_Q:
                    case I_VI2C_Q:
                    case I_VI2F_P:
                    case I_VI2F_Q:
                    case I_VI2F_S:
                    case I_VI2F_T:
                    case I_VI2S_P:
                    case I_VI2S_Q:
                    case I_VI2UC_Q:
                    case I_VI2US_P:
                    case I_VI2US_Q:
                    case I_VIDT_P:
                    case I_VIDT_Q:
                    case I_VIIM_S:
                    case I_VLGB_S:
                    case I_VLOG2_P:
                    case I_VLOG2_Q:
                    case I_VLOG2_S:
                    case I_VLOG2_T:
                    case I_VMAX_P:
                    case I_VMAX_Q:
                    case I_VMAX_S:
                    case I_VMAX_T:
                    case I_VMFVC:
                    case I_VMIDT_P:
                    case I_VMIDT_Q:
                    case I_VMIDT_T:
                    case I_VMIN_P:
                    case I_VMIN_Q:
                    case I_VMIN_S:
                    case I_VMIN_T:
                    case I_VMMOV_P:
                    case I_VMMOV_Q:
                    case I_VMMOV_T:
                    case I_VMMUL_P:
                    case I_VMMUL_Q:
                    case I_VMMUL_T:
                    case I_VMONE_P:
                    case I_VMONE_Q:
                    case I_VMONE_T:
                    case I_VMOV_P:
                    case I_VMOV_Q:
                    case I_VMOV_S:
                    case I_VMOV_T:
                    case I_VMSCL_P:
                    case I_VMSCL_Q:
                    case I_VMSCL_T:
                    case I_VMTVC:
                    case I_VMUL_P:
                    case I_VMUL_Q:
                    case I_VMUL_S:
                    case I_VMUL_T:
                    case I_VMZERO_P:
                    case I_VMZERO_Q:
                    case I_VMZERO_T:
                    case I_VNEG_P:
                    case I_VNEG_Q:
                    case I_VNEG_S:
                    case I_VNEG_T:
                    case I_VNOP:
                    case I_VNRCP_P:
                    case I_VNRCP_Q:
                    case I_VNRCP_S:
                    case I_VNRCP_T:
                    case I_VNSIN_P:
                    case I_VNSIN_Q:
                    case I_VNSIN_S:
                    case I_VNSIN_T:
                    case I_VOCP_P:
                    case I_VOCP_Q:
                    case I_VOCP_S:
                    case I_VOCP_T:
                    case I_VONE_P:
                    case I_VONE_Q:
                    case I_VONE_S:
                    case I_VONE_T:
                    case I_VPFXD:
                    case I_VPFXS:
                    case I_VPFXT:
                    case I_VQMUL_Q:
                    case I_VRCP_P:
                    case I_VRCP_Q:
                    case I_VRCP_S:
                    case I_VRCP_T:
                    case I_VREXP2_P:
                    case I_VREXP2_Q:
                    case I_VREXP2_S:
                    case I_VREXP2_T:
                    case I_VRNDF1_P:
                    case I_VRNDF1_Q:
                    case I_VRNDF1_S:
                    case I_VRNDF1_T:
                    case I_VRNDF2_P:
                    case I_VRNDF2_Q:
                    case I_VRNDF2_S:
                    case I_VRNDF2_T:
                    case I_VRNDI_P:
                    case I_VRNDI_Q:
                    case I_VRNDI_S:
                    case I_VRNDI_T:
                    case I_VRNDS_S:
                    case I_VROT_P:
                    case I_VROT_Q:
                    case I_VROT_T:
                    case I_VRSQ_P:
                    case I_VRSQ_Q:
                    case I_VRSQ_S:
                    case I_VRSQ_T:
                    case I_VS2I_P:
                    case I_VS2I_S:
                    case I_VSAT0_P:
                    case I_VSAT0_Q:
                    case I_VSAT0_S:
                    case I_VSAT0_T:
                    case I_VSAT1_P:
                    case I_VSAT1_Q:
                    case I_VSAT1_S:
                    case I_VSAT1_T:
                    case I_VSBN_S:
                    case I_VSBZ_S:
                    case I_VSCL_P:
                    case I_VSCL_Q:
                    case I_VSCL_T:
                    case I_VSCMP_P:
                    case I_VSCMP_Q:
                    case I_VSCMP_S:
                    case I_VSCMP_T:
                    case I_VSGE_P:
                    case I_VSGE_Q:
                    case I_VSGE_S:
                    case I_VSGE_T:
                    case I_VSGN_P:
                    case I_VSGN_Q:
                    case I_VSGN_S:
                    case I_VSGN_T:
                    case I_VSIN_P:
                    case I_VSIN_Q:
                    case I_VSIN_S:
                    case I_VSIN_T:
                    case I_VSLT_P:
                    case I_VSLT_Q:
                    case I_VSLT_S:
                    case I_VSLT_T:
                    case I_VSOCP_P:
                    case I_VSOCP_S:
                    case I_VSQRT_P:
                    case I_VSQRT_Q:
                    case I_VSQRT_S:
                    case I_VSQRT_T:
                    case I_VSRT1_Q:
                    case I_VSRT2_Q:
                    case I_VSRT3_Q:
                    case I_VSRT4_Q:
                    case I_VSUB_P:
                    case I_VSUB_Q:
                    case I_VSUB_S:
                    case I_VSUB_T:
                    case I_VSYNC:
                    case I_VT4444_Q:
                    case I_VT5551_Q:
                    case I_VT5650_Q:
                    case I_VTFM2_P:
                    case I_VTFM3_T:
                    case I_VTFM4_Q:
                    case I_VUS2I_P:
                    case I_VUS2I_S:
                    case I_VWB_Q:
                    case I_VWBN_S:
                    case I_VZERO_P:
                    case I_VZERO_Q:
                    case I_VZERO_S:
                    case I_VZERO_T:
                    case I_MFVME:
                    case I_MTVME:
                    default:
                        _(std::make_unique<core::ir::InlineAssembly>());
                        break;
                    }

                    return _.basicBlock();
                }

                const nc::arch::allegrex::allegrex_instruction *disassemble(const AllegrexInstruction *instruction, allegrex_operand operand[8]) {
                    auto opcode = *((int32_t *)instruction->bytes());
                    auto insn = allegrex_decode_instruction(opcode, false);
                    if (insn) {
                        allegrex_decode_operands(insn, opcode, instruction->addr(), operand);
                        return insn;
                    }
                    return nullptr;
                }
            };

            AllegrexInstructionAnalyzer::AllegrexInstructionAnalyzer(const AllegrexArchitecture *architecture) :
                impl_(std::make_unique<AllegrexInstructionAnalyzerImpl>(architecture))
            {
            }

            AllegrexInstructionAnalyzer::~AllegrexInstructionAnalyzer() {}

            void AllegrexInstructionAnalyzer::doCreateStatements(const core::arch::Instruction *instruction, core::ir::Program *program) {
                impl_->setInstructions(instructions());
                impl_->createStatements(checked_cast<const AllegrexInstruction *>(instruction), program);
            }
        }
    }
} // namespace nc::arch::allegrex

/* vim:set et sts=4 sw=4: */
