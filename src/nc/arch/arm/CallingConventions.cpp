/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "CallingConventions.h"

#include <nc/common/make_unique.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "ArmArchitecture.h"
#include "ArmRegisters.h"

namespace nc {
namespace arch {
namespace arm {

DefaultCallingConvention::DefaultCallingConvention():
    core::ir::calling::Convention(QLatin1String("Default"))
{
    setStackPointer(ArmRegisters::sp()->memoryLocation());

    setFirstArgumentOffset(0);
    setArgumentAlignment(32);

    std::vector<core::ir::MemoryLocation> args;
    args.push_back(ArmRegisters::r0()->memoryLocation());
    args.push_back(ArmRegisters::r1()->memoryLocation());
    args.push_back(ArmRegisters::r2()->memoryLocation());
    args.push_back(ArmRegisters::r3()->memoryLocation());
    addArgumentGroup(std::move(args));

    addReturnValueLocation(ArmRegisters::r0()->memoryLocation());

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(ArmRegisters::lr()->memoryLocation()),
        std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::RETURN_ADDRESS, ArmRegisters::lr()->size())
    ));
}

}}} // namespace nc::arch::arm

/* vim:set et sts=4 sw=4: */
