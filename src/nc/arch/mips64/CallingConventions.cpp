/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "CallingConventions.h"

#include <nc/common/make_unique.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "Mips64Architecture.h"
#include "Mips64Registers.h"

namespace nc {
namespace arch {
namespace mips64 {

n32CallingConvention::n32CallingConvention(const Mips64Architecture *architecture_):
    core::ir::calling::Convention(QLatin1String("n32"))
{
    setStackPointer(Mips64Registers::sp()->memoryLocation());

    setFirstArgumentOffset(0);
    setArgumentAlignment(architecture_->bitness());

	/* Regular registers */
    std::vector<core::ir::MemoryLocation> args;
    args.push_back(Mips64Registers::a0()->memoryLocation());
    args.push_back(Mips64Registers::a1()->memoryLocation());
    args.push_back(Mips64Registers::a2()->memoryLocation());
    args.push_back(Mips64Registers::a3()->memoryLocation());
    args.push_back(Mips64Registers::t0()->memoryLocation());
    args.push_back(Mips64Registers::t1()->memoryLocation());
    args.push_back(Mips64Registers::t2()->memoryLocation());
    args.push_back(Mips64Registers::t3()->memoryLocation());
    addArgumentGroup(std::move(args));

	/* FP registers */
    std::vector<core::ir::MemoryLocation> fpArgs;
    fpArgs.push_back(Mips64Registers::f12()->memoryLocation());
    fpArgs.push_back(Mips64Registers::f13()->memoryLocation());
	fpArgs.push_back(Mips64Registers::f14()->memoryLocation());
	fpArgs.push_back(Mips64Registers::f15()->memoryLocation());
	fpArgs.push_back(Mips64Registers::f16()->memoryLocation());
	fpArgs.push_back(Mips64Registers::f17()->memoryLocation());
	fpArgs.push_back(Mips64Registers::f18()->memoryLocation());
	fpArgs.push_back(Mips64Registers::f19()->memoryLocation());
    addArgumentGroup(std::move(fpArgs));

	/* Regular registers */
    addReturnValueLocation(Mips64Registers::v0()->memoryLocation());
    addReturnValueLocation(Mips64Registers::v1()->memoryLocation());
 
	/* FP registers */
    addReturnValueLocation(Mips64Registers::f0()->memoryLocation());
    addReturnValueLocation(Mips64Registers::f2()->memoryLocation());
    
    /* Regular registers */
    /*addNonVolatileRegisterLocation(Mips64Registers::s0()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::s1()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::s2()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::s3()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::s4()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::s5()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::s6()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::s7()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::fp()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::sp()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::ra()->memoryLocation());*/
    addNonVolatileRegisterLocation(Mips64Registers::gp()->memoryLocation());
    
    /* FP registers */
	/*addNonVolatileRegisterLocation(Mips64Registers::f20()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::f21()->memoryLocation());
	addNonVolatileRegisterLocation(Mips64Registers::f22()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::f23()->memoryLocation());
	addNonVolatileRegisterLocation(Mips64Registers::f24()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::f25()->memoryLocation());
	addNonVolatileRegisterLocation(Mips64Registers::f26()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::f27()->memoryLocation());
	addNonVolatileRegisterLocation(Mips64Registers::f28()->memoryLocation());
    addNonVolatileRegisterLocation(Mips64Registers::f29()->memoryLocation());
	addNonVolatileRegisterLocation(Mips64Registers::f30()->memoryLocation());*/

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(Mips64Registers::ra()->memoryLocation()),
        std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::RETURN_ADDRESS, Mips64Registers::ra()->size())
    ));
}

}}} // namespace nc::arch::mips64

/* vim:set et sts=4 sw=4: */
