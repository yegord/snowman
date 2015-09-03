/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "AllegrexCallingConventions.h"

#include <nc/common/make_unique.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "AllegrexArchitecture.h"
#include "AllegrexRegisters.h"

namespace nc {
namespace arch {
namespace allegrex {

DefaultCallingConvention::DefaultCallingConvention(const AllegrexArchitecture *architecture_):
    core::ir::calling::Convention(QLatin1String("Default"))
{
    setStackPointer(AllegrexRegisters::sp()->memoryLocation());

    setFirstArgumentOffset(0);
    setArgumentAlignment(architecture_->bitness());

    std::vector<core::ir::MemoryLocation> args;
    args.push_back(AllegrexRegisters::a0()->memoryLocation());
    args.push_back(AllegrexRegisters::a1()->memoryLocation());
    args.push_back(AllegrexRegisters::a2()->memoryLocation());
    args.push_back(AllegrexRegisters::a3()->memoryLocation());
    args.push_back(AllegrexRegisters::t0()->memoryLocation());
    args.push_back(AllegrexRegisters::t1()->memoryLocation());
    args.push_back(AllegrexRegisters::t2()->memoryLocation());
    args.push_back(AllegrexRegisters::t3()->memoryLocation());
    addArgumentGroup(std::move(args));

    addReturnValueLocation(AllegrexRegisters::v0()->memoryLocation());
    addReturnValueLocation(AllegrexRegisters::v1()->memoryLocation());

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(AllegrexRegisters::ra()->memoryLocation()),
        std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::RETURN_ADDRESS, AllegrexRegisters::ra()->size())
    ));
}

}}} // namespace nc::arch::allegrex

/* vim:set et sts=4 sw=4: */
