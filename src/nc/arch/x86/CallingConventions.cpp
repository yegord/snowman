/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "CallingConventions.h"

#include <nc/common/make_unique.h>

#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>

#include "X86Architecture.h"
#include "X86Registers.h"

namespace nc {
namespace arch {
namespace x86 {

AMD64CallingConvention::AMD64CallingConvention(const X86Architecture *architecture):
    Convention(QLatin1String("amd64"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(architecture->bitness());
    setArgumentAlignment(architecture->bitness());

    /* Used for integer and pointer arguments. */
    std::vector<core::ir::MemoryLocation> scalarArgs;
    scalarArgs.push_back(X86Registers::rdi()->memoryLocation());
    scalarArgs.push_back(X86Registers::rsi()->memoryLocation());
    scalarArgs.push_back(X86Registers::rdx()->memoryLocation());
    scalarArgs.push_back(X86Registers::rcx()->memoryLocation());
    scalarArgs.push_back(X86Registers::r8()->memoryLocation());
    scalarArgs.push_back(X86Registers::r9()->memoryLocation());
    addArgumentGroup(std::move(scalarArgs));

    /* Used for floating-point arguments. */
    std::vector<core::ir::MemoryLocation> fpArgs;
    fpArgs.push_back(X86Registers::xmm0()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm1()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm2()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm3()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm4()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm5()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm6()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm7()->memoryLocation());
    addArgumentGroup(std::move(fpArgs));

    addReturnValueLocation(X86Registers::rax()->memoryLocation());
    addReturnValueLocation(X86Registers::xmm0()->memoryLocation());

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(core::ir::MemoryLocation(core::ir::MemoryDomain::STACK, 0, architecture->bitness())),
        std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::RETURN_ADDRESS, architecture->bitness())
    ));
    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(X86Registers::df()->memoryLocation()),
        std::make_unique<core::ir::Constant>(SizedValue(X86Registers::df()->size(), 0))
    ));
}

Microsoft64CallingConvention::Microsoft64CallingConvention(const X86Architecture *architecture):
    Convention(QLatin1String("microsoft64"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(architecture->bitness());
    setArgumentAlignment(architecture->bitness());

    /* Used for integer and pointer arguments. */
    std::vector<core::ir::MemoryLocation> scalarArgs;
    scalarArgs.push_back(X86Registers::rcx()->memoryLocation());
    scalarArgs.push_back(X86Registers::rdx()->memoryLocation());
    scalarArgs.push_back(X86Registers::r8()->memoryLocation());
    scalarArgs.push_back(X86Registers::r9()->memoryLocation());
    addArgumentGroup(std::move(scalarArgs));

    /* Used for floating-point arguments. */
    std::vector<core::ir::MemoryLocation> fpArgs;
    fpArgs.push_back(X86Registers::xmm0()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm1()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm2()->memoryLocation());
    fpArgs.push_back(X86Registers::xmm3()->memoryLocation());
    addArgumentGroup(std::move(fpArgs));

    addReturnValueLocation(X86Registers::rax()->memoryLocation());
    addReturnValueLocation(X86Registers::xmm0()->memoryLocation());

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(core::ir::MemoryLocation(core::ir::MemoryDomain::STACK, 0, architecture->bitness())),
        std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::RETURN_ADDRESS, architecture->bitness())
    ));
    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(X86Registers::df()->memoryLocation()),
        std::make_unique<core::ir::Constant>(SizedValue(X86Registers::df()->size(), 0))
    ));
}

Cdecl32CallingConvention::Cdecl32CallingConvention(const X86Architecture *architecture):
    Convention(QLatin1String("cdecl32"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(architecture->bitness());
    setArgumentAlignment(architecture->bitness());

    addArgumentGroup(std::vector<core::ir::MemoryLocation>(1, X86Registers::ecx()->memoryLocation()));
    addArgumentGroup(std::vector<core::ir::MemoryLocation>());

    addReturnValueLocation(X86Registers::eax()->memoryLocation());
    addReturnValueLocation(X86Registers::st0()->memoryLocation());

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(core::ir::MemoryLocation(core::ir::MemoryDomain::STACK, 0, architecture->bitness())),
        std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::RETURN_ADDRESS, architecture->bitness())
    ));
    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(X86Registers::df()->memoryLocation()),
        std::make_unique<core::ir::Constant>(SizedValue(X86Registers::df()->size(), 0))
    ));
}

Cdecl16CallingConvention::Cdecl16CallingConvention(const X86Architecture *architecture):
    Convention(QLatin1String("cdecl16"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(architecture->bitness());
    setArgumentAlignment(architecture->bitness());

    addReturnValueLocation(X86Registers::ax()->memoryLocation());
    addReturnValueLocation(X86Registers::st0()->memoryLocation());

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(core::ir::MemoryLocation(core::ir::MemoryDomain::STACK, 0, architecture->bitness())),
        std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::RETURN_ADDRESS, architecture->bitness())
    ));
    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(X86Registers::df()->memoryLocation()),
        std::make_unique<core::ir::Constant>(SizedValue(X86Registers::df()->size(), 0))
    ));
}

Stdcall32CallingConvention::Stdcall32CallingConvention(const X86Architecture *architecture):
    Convention(QLatin1String("stdcall32"))
{
    setStackPointer(architecture->stackPointer()->memoryLocation());

    setFirstArgumentOffset(architecture->bitness());
    setArgumentAlignment(architecture->bitness());
    setCalleeCleanup(true);

    addArgumentGroup(std::vector<core::ir::MemoryLocation>(1, X86Registers::ecx()->memoryLocation()));
    addArgumentGroup(std::vector<core::ir::MemoryLocation>());

    addReturnValueLocation(X86Registers::ax()->memoryLocation());
    addReturnValueLocation(X86Registers::st0()->memoryLocation());

    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(core::ir::MemoryLocation(core::ir::MemoryDomain::STACK, 0, architecture->bitness())),
        std::make_unique<core::ir::Intrinsic>(core::ir::Intrinsic::RETURN_ADDRESS, architecture->bitness())
    ));
    addEnterStatement(std::make_unique<core::ir::Assignment>(
        std::make_unique<core::ir::MemoryLocationAccess>(X86Registers::df()->memoryLocation()),
        std::make_unique<core::ir::Constant>(SizedValue(X86Registers::df()->size(), 0))
    ));
}

} // namespace x86
} // namespace arch
} // namespace nc

/* vim:set et ts=4 sw=4: */
