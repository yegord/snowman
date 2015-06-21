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

DefaultCallingConvention::DefaultCallingConvention():
    core::ir::calling::Convention(QLatin1String("Default"))
{
    setStackPointer(MipsRegisters::sp()->memoryLocation());

    setFirstArgumentOffset(0);
    setArgumentAlignment(32);

    std::vector<core::ir::MemoryLocation> args;
    args.push_back(MipsRegisters::a0()->memoryLocation());
    args.push_back(MipsRegisters::a1()->memoryLocation());
    args.push_back(MipsRegisters::a2()->memoryLocation());
    args.push_back(MipsRegisters::a3()->memoryLocation());
    addArgumentGroup(std::move(args));

    addReturnValueLocation(MipsRegisters::r0()->memoryLocation());

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(MipsRegisters::lr()->memoryLocation()),
        std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::RETURN_ADDRESS, MipsRegisters::lr()->size())
    ));
}

}}} // namespace nc::arch::mips

/* vim:set et sts=4 sw=4: */
