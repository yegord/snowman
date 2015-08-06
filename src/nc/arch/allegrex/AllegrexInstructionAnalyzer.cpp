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

                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, sp);
                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, gp);
                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, ra);
                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, hilo);
                NC_DEFINE_REGISTER_EXPRESSION(AllegrexRegisters, lo);

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

                    createStatements(_, instruction, program);
                }

            private:
                void createStatements(AllegrexExpressionFactoryCallback & _, const AllegrexInstruction *instruction, core::ir::Program *program) {
                    using namespace core::irgen::expressions;

                    auto instr = disassemble(instruction);
                }

                const nc::arch::allegrex::allegrex_instruction *disassemble(const AllegrexInstruction *instruction) {
                    return allegrex_decode_instruction(*((int32_t *)instruction->bytes()), true);
                }

                static const core::arch::Register *getRegister(int reg) {
                    switch (reg) {

#if 0
#define REG(uppercase, lowercase) \
            case MIPS_REG_##uppercase: return AllegrexRegisters::lowercase()
                        REG(ZERO, zero);
                        REG(AT, at);
                        REG(V0, v0);
                        REG(V1, v1);
                        REG(A0, a0);
                        REG(A1, a1);
                        REG(A2, a2);
                        REG(A3, a3);
                        REG(T0, t0);
                        REG(T1, t1);
                        REG(T2, t2);
                        REG(T3, t3);
                        REG(T4, t4);
                        REG(T5, t5);
                        REG(T6, t6);
                        REG(T7, t7);
                        REG(S0, s0);
                        REG(S1, s1);
                        REG(S2, s2);
                        REG(S3, s3);
                        REG(S4, s4);
                        REG(S5, s5);
                        REG(S6, s6);
                        REG(S7, s7);
                        REG(T8, t8);
                        REG(T9, t9);
                        REG(K0, k0);
                        REG(K1, k1);
                        REG(GP, gp);
                        REG(SP, sp);
                        REG(FP, fp); /*REG(S8, s8);*/
                        REG(RA, ra);

                        REG(F0, f0);
                        REG(F1, f1);
                        REG(F2, f2);
                        REG(F3, f3);
                        REG(F4, f4);
                        REG(F5, f5);
                        REG(F6, f6);
                        REG(F7, f7);
                        REG(F8, f8);
                        REG(F9, f9);
                        REG(F10, f10);
                        REG(F11, f11);
                        REG(F12, f12);
                        REG(F13, f13);
                        REG(F14, f14);
                        REG(F15, f15);
                        REG(F16, f16);
                        REG(F17, f17);
                        REG(F18, f18);
                        REG(F19, f19);
                        REG(F20, f20);
                        REG(F21, f21);
                        REG(F22, f22);
                        REG(F23, f23);
                        REG(F24, f24);
                        REG(F25, f25);
                        REG(F26, f26);
                        REG(F27, f27);
                        REG(F28, f28);
                        REG(F29, f29);
                        REG(F30, f30);
                        REG(F31, f31);

                        REG(HI, hi);
                        REG(LO, lo);
#undef REG
#endif
                    default:
                        throw core::irgen::InvalidInstructionException(tr("Invalid register number: %1").arg(reg));
                    }
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
