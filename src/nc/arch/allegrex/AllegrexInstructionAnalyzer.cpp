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

                std::unique_ptr<core::ir::Term> gpr(int index) const {
                    auto &reg = gpr_(index)->memoryLocation();
                    return std::make_unique<core::ir::MemoryLocationAccess>(reg.resized(32));
                }

                std::unique_ptr<core::ir::Term> fpr(int index) const {
                    auto &reg = fpr_(index)->memoryLocation();
                    return std::make_unique<core::ir::MemoryLocationAccess>(reg.resized(32));
                }

                std::unique_ptr<core::ir::Term> vpr(int index) const {
                    throw core::irgen::InvalidInstructionException(tr("Invalid VPR number: %1").arg(index));
                }


                void createStatements(AllegrexExpressionFactoryCallback & _, const AllegrexInstruction *instruction, core::ir::Program *program) {
                    using namespace core::irgen::expressions;

                    auto insn = disassemble(instruction);
                    if (insn == nullptr)
                        return;

                    switch (insn->id)
                    {
                    case I_ADD:
                    case I_ADDI:
                    case I_ADDIU:
                    case I_ADDU:
                    case I_AND:
                    case I_ANDI:
                    case I_BEQ:
                    case I_BEQL:
                    case I_BGEZ:
                    case I_BGEZAL:
                    case I_BGEZL:
                    case I_BGTZ:
                    case I_BGTZL:
                    case I_BITREV:
                    case I_BLEZ:
                    case I_BLEZL:
                    case I_BLTZ:
                    case I_BLTZL:
                    case I_BLTZAL:
                    case I_BLTZALL:
                    case I_BNE:
                    case I_BNEL:
                    case I_BREAK:
                    case I_CACHE:
                    case I_CFC0:
                    case I_CLO:
                    case I_CLZ:
                    case I_CTC0:
                    case I_MAX:
                    case I_MIN:
                    case I_DBREAK:
                    case I_DIV:
                    case I_DIVU:
                    case I_DRET:
                    case I_ERET:
                    case I_EXT:
                    case I_INS:
                    case I_J:
                    case I_JR:
                    case I_JALR:
                    case I_JAL:
                    case I_LB:
                    case I_LBU:
                    case I_LH:
                    case I_LHU:
                    case I_LL:
                    case I_LUI:
                    case I_LW:
                    case I_LWL:
                    case I_LWR:
                    case I_MADD:
                    case I_MADDU:
                    case I_MFC0:
                    case I_MFDR:
                    case I_MFHI:
                    case I_MFIC:
                    case I_MFLO:
                    case I_MOVN:
                    case I_MOVZ:
                    case I_MSUB:
                    case I_MSUBU:
                    case I_MTC0:
                    case I_MTDR:
                    case I_MTIC:
                    case I_HALT:
                    case I_MTHI:
                    case I_MTLO:
                    case I_MULT:
                    case I_MULTU:
                    case I_NOR:
                    case I_OR:
                    case I_ORI:
                    case I_ROTR:
                    case I_ROTV:
                    case I_SEB:
                    case I_SEH:
                    case I_SB:
                    case I_SC:
                    case I_SH:
                    case I_SLLV:
                    case I_SLL:
                    case I_SLT:
                    case I_SLTI:
                    case I_SLTIU:
                    case I_SLTU:
                    case I_SRA:
                    case I_SRAV:
                    case I_SRLV:
                    case I_SRL:
                    case I_SW:
                    case I_SWL:
                    case I_SWR:
                    case I_SUB:
                    case I_SUBU:
                    case I_SYNC:
                    case I_SYSCALL:
                    case I_XOR:
                    case I_XORI:
                    case I_WSBH:
                    case I_WSBW:
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
