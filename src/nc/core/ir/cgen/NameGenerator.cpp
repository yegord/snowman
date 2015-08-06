/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "NameGenerator.h"

#include <nc/common/Foreach.h>
#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Registers.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Relocation.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/MemoryLocation.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/calling/CalleeId.h>
#include <nc/core/mangling/Demangler.h>

namespace nc {
namespace core {
namespace ir {
namespace cgen {

NameAndComment NameGenerator::getFunctionName(const calling::CalleeId &calleeId) const {
    if (calleeId.entryAddress()) {
        return getFunctionName(*calleeId.entryAddress());
    } else if (calleeId.function()) {
        return getFunctionName(calleeId.function());
    }

    return NameAndComment();
}

NameAndComment NameGenerator::getFunctionName(const Function *function) const {
    if (auto entry = function->entry()) {
        if (const auto &address = entry->address()) {
            return getFunctionName(*address);
        }
    }
    return tr("fun_noaddr_%1").arg(reinterpret_cast<uintptr_t>(function), 0, 16);
}

NameAndComment NameGenerator::getFunctionName(ByteAddr addr) const {
    if (auto symbol = image_.getSymbol(addr)) {
        if (auto result = getFunctionName(symbol)) {
            return result;
        }
    }
    return tr("fun_%1").arg(addr, 0, 16);
}

NameAndComment NameGenerator::getFunctionName(const image::Symbol *symbol) const {
    assert(symbol != nullptr);

    QString name = cleanName(symbol->name());
    QString comment;

    if (name != symbol->name()) {
        comment += symbol->name();
        comment += '\n';
    }

    auto demangledName = image_.demangler()->demangle(symbol->name());
    if (demangledName.contains('(')) {
        comment += demangledName;
        comment += '\n';
    }

    return NameAndComment(std::move(name), comment.trimmed());
}

NameAndComment NameGenerator::getGlobalVariableName(const MemoryLocation &memoryLocation) const {
    if (auto reg = image_.platform().architecture()->registers()->getRegister(memoryLocation)) {
        return reg->lowercaseName();
    }

    if (memoryLocation.domain() == MemoryDomain::MEMORY && (memoryLocation.addr() % CHAR_BIT == 0)) {
        return getGlobalVariableName(memoryLocation.addr() / CHAR_BIT);
    }

    return NameAndComment();
}

NameAndComment NameGenerator::getGlobalVariableName(ByteAddr addr) const {
    if (auto relocation = image_.getRelocation(addr)) {
        if (auto result = getFunctionName(relocation->symbol())) {
            return result;
        }
    }
    if (auto symbol = image_.getSymbol(addr)) {
        if (auto result = getGlobalVariableName(symbol)) {
            return result;
        }
    }
    return tr("g%1").arg(addr, 0, 16);
}

NameAndComment NameGenerator::getGlobalVariableName(const image::Symbol *symbol) const {
    assert(symbol != nullptr);

    auto name = cleanName(symbol->name());
    if (name == symbol->name()) {
        return name;
    } else {
        return NameAndComment(name, symbol->name());
    }
}

NameAndComment NameGenerator::getLocalVariableName(const MemoryLocation &memoryLocation, std::size_t serial) const {
    QString name;

    if (auto reg = image_.platform().architecture()->registers()->getRegister(memoryLocation)) {
        name = reg->lowercaseName();
        assert(!name.isEmpty());

        if (name[name.size() - 1].isDigit()) {
            name.append('_');
        }
    } else {
        name = tr("v");
    }

    return tr("%1%2").arg(name).arg(serial);
}

NameAndComment NameGenerator::getArgumentName(const Term *term, std::size_t serial) const {
    if (auto access = term->asMemoryLocationAccess()) {
        if (auto reg = image_.platform().architecture()->registers()->getRegister(access->memoryLocation())) {
            return reg->lowercaseName();
        }
    }
    return tr("a%1").arg(serial);
}

QString NameGenerator::cleanName(const QString &name) {
    QString result;
    result.reserve(name.size());

    bool skipped = true;
    foreach (QChar c, name) {
        if (c.isLetterOrNumber() || c == '_') {
            result += c;
            skipped = false;
        } else if (!skipped) {
            result += '_';
            skipped = true;
        }
    }

    return result;
}

}}}} // namespace nc::core::ir::cgen

/* vim:set et sts=4 sw=4: */
