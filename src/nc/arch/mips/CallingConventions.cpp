/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "CallingConventions.h"

#include <nc/common/make_unique.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "MipsArchitecture.h"
#include "MipsRegisters.h"

namespace nc {
namespace arch {
namespace mips {

DefaultCallingConvention::DefaultCallingConvention(const MipsArchitecture *architecture_):
    core::ir::calling::Convention(QLatin1String("Default"))
{
    setStackPointer(MipsRegisters::sp()->memoryLocation());

    setFirstArgumentOffset(0);
    setArgumentAlignment(architecture_->bitness());

	/* Regular registers */
    std::vector<core::ir::MemoryLocation> args;
    args.push_back(MipsRegisters::a0()->memoryLocation());
    args.push_back(MipsRegisters::a1()->memoryLocation());
    args.push_back(MipsRegisters::a2()->memoryLocation());
    args.push_back(MipsRegisters::a3()->memoryLocation());
    addArgumentGroup(std::move(args));

	/* FP registers */
    std::vector<core::ir::MemoryLocation> fpArgs;
    fpArgs.push_back(MipsRegisters::f12()->memoryLocation());
    fpArgs.push_back(MipsRegisters::f13()->memoryLocation());
	fpArgs.push_back(MipsRegisters::f14()->memoryLocation());
    addArgumentGroup(std::move(fpArgs));

	/* Regular registers */
    addReturnValueLocation(MipsRegisters::v0()->memoryLocation());
    addReturnValueLocation(MipsRegisters::v1()->memoryLocation());
 
	/* FP registers */
    addReturnValueLocation(MipsRegisters::f0()->memoryLocation());
    addReturnValueLocation(MipsRegisters::f1()->memoryLocation());
    
    /* Regular registers */
    addNonVolatileRegisterLocation(MipsRegisters::s0()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::s1()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::s2()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::s3()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::s4()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::s5()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::s6()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::s7()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::fp()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::sp()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::gp()->memoryLocation());
    
    /* FP registers */
	addNonVolatileRegisterLocation(MipsRegisters::f20()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::f21()->memoryLocation());
	addNonVolatileRegisterLocation(MipsRegisters::f22()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::f23()->memoryLocation());
	addNonVolatileRegisterLocation(MipsRegisters::f24()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::f25()->memoryLocation());
	addNonVolatileRegisterLocation(MipsRegisters::f26()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::f27()->memoryLocation());
	addNonVolatileRegisterLocation(MipsRegisters::f28()->memoryLocation());
    addNonVolatileRegisterLocation(MipsRegisters::f29()->memoryLocation());
	addNonVolatileRegisterLocation(MipsRegisters::f30()->memoryLocation());

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(MipsRegisters::ra()->memoryLocation()),
        std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::RETURN_ADDRESS, MipsRegisters::ra()->size())
    ));
}

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */
