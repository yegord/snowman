/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Driver.h"

#include <QFile>

#include <nc/common/Foreach.h>
#include <nc/common/Exception.h>

#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/Disassembler.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>
#include <nc/core/input/Parser.h>
#include <nc/core/input/ParserRepository.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>

#include "Context.h"
#include "MasterAnalyzer.h"

namespace nc {
namespace core {

void Driver::parse(Context &context, const QString &filename) {
    QFile source(filename);

    if (!source.open(QIODevice::ReadOnly)) {
        throw nc::Exception(tr("Could not open file \"%1\" for reading.").arg(filename));
    }

    context.logToken().info(tr("Choosing a parser for %1...").arg(filename));

    const input::Parser *suitableParser = nullptr;

    foreach(const input::Parser *parser, input::ParserRepository::instance()->parsers()) {
        context.logToken().info(tr("Trying %1 parser...").arg(parser->name()));
        if (parser->canParse(&source)) {
            suitableParser = parser;
            break;
        }
    }

    if (!suitableParser) {
        context.logToken().error(tr("No suitable parser found."));
        throw nc::Exception(tr("File %1 has unknown format.").arg(filename));
    }

    context.logToken().info(tr("Parsing using %1 parser...").arg(suitableParser->name()));

    suitableParser->parse(&source, context.image().get(), context.logToken());

    context.logToken().info(tr("Parsing completed."));
}

void Driver::disassemble(Context &context) {
    context.logToken().info(tr("Disassemble code sections."));

    foreach (auto section, context.image()->sections()) {
        if (section->isCode()) {
            disassemble(context, section);
        }
    }
}

void Driver::disassemble(Context &context, const image::Section *section) {
    assert(section != nullptr);

    context.logToken().info(tr("Disassemble section %1...").arg(section->name()));

    disassemble(context, section, section->addr(), section->endAddr());
}

void Driver::disassemble(Context &context, const image::ByteSource *source, ByteAddr begin, ByteAddr end) {
    assert(source != nullptr);

    context.logToken().info(tr("Disassemble addresses from 0x%2 to 0x%3...").arg(begin, 0, 16).arg(end, 0, 16));

    try {
        auto newInstructions = std::make_shared<arch::Instructions>(*context.instructions());

        context.image()->architecture()->createDisassembler()->disassemble(
            source,
            begin,
            end,
            [&](std::shared_ptr<arch::Instruction> instr){ newInstructions->add(std::move(instr)); },
            context.cancellationToken());

        context.setInstructions(newInstructions);

        context.logToken().info(tr("Disassembly completed."));
    } catch (const CancellationException &) {
        context.logToken().info(tr("Disassembly canceled."));
    }
}

void Driver::decompile(Context &context) {
    try {
        context.image()->architecture()->masterAnalyzer()->decompile(context);
    } catch (const CancellationException &) {
        context.logToken().info(tr("Decompilation canceled."));
        throw;
    }
}

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
