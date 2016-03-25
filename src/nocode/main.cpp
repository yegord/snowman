/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include <nc/config.h>

#include <nc/common/Branding.h>
#include <nc/common/Exception.h>
#include <nc/common/Foreach.h>
#include <nc/common/StreamLogger.h>
#include <nc/common/Unreachable.h>

#include <nc/core/Context.h>
#include <nc/core/Driver.h>
#include <nc/core/arch/Architecture.h>
#include <nc/core/arch/ArchitectureRepository.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/arch/Instructions.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>
#include <nc/core/input/Parser.h>
#include <nc/core/input/ParserRepository.h>
#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Function.h>
#include <nc/core/ir/Functions.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/Statements.h>
#include <nc/core/ir/Terms.h>
#include <nc/core/ir/cflow/Graphs.h>
#include <nc/core/likec/Tree.h>

#include <QCoreApplication>
#include <QFile>
#include <QStringList>
#include <QTextStream>

const char *self = "nocode";

QTextStream qin(stdin, QIODevice::ReadOnly);
QTextStream qout(stdout, QIODevice::WriteOnly);
QTextStream qerr(stderr, QIODevice::WriteOnly);

template<class T>
void openFileForWritingAndCall(const QString &filename, T functor) {
    if (filename.isEmpty()) {
        return;
    } else if (filename == "-") {
        functor(qout);
    } else {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            throw nc::Exception("could not open file for writing");
        }
        QTextStream out(&file);
        functor(out);
    }
}

void printSections(nc::core::Context &context, QTextStream &out) {
    foreach (auto section, context.image()->sections()) {
        QString flags;
        if (section->isReadable()) {
            flags += QLatin1String("r");
        }
        if (section->isWritable()) {
            flags += QLatin1String("w");
        }
        if (section->isExecutable()) {
            flags += QLatin1String("x");
        }
        if (section->isCode()) {
            flags += QLatin1String(",code");
        }
        if (section->isData()) {
            flags += QLatin1String(",data");
        }
        if (section->isBss()) {
            flags += QLatin1String(",bss");
        }
        out << QString(QLatin1String("section name = '%1', start = 0x%2, size = 0x%3, flags = %4"))
            .arg(section->name()).arg(section->addr(), 0, 16).arg(section->size(), 0, 16).arg(flags) << endl;
    }
    auto entrypoint = context.image()->entrypoint();
    if (entrypoint) {
        out << QString(QLatin1String("entry point = 0x%1")).arg(*entrypoint, 0, 16) << endl;
    }
}

void printSymbols(nc::core::Context &context, QTextStream &out) {
    foreach (const auto *symbol, context.image()->symbols()) {
        QString value;
        if (symbol->value()) {
            value = QString("%1").arg(*symbol->value(), 0, 16);
        } else {
            value = QLatin1String("Undefined");
        }
        out << QString("symbol name = '%1', type = %2, value = 0x%3, section = %4")
            .arg(symbol->name()).arg(symbol->type().getName()).arg(value)
            .arg(symbol->section() ? symbol->section()->name() : QString()) << endl;
    }
}

void printRegionGraphs(nc::core::Context &context, QTextStream &out) {
    out << "digraph Functions { compound=true; " << endl;
    foreach (const auto *function, context.functions()->list()) {
        context.graphs()->at(function)->print(out);
    }
    out << "}" << endl;
}

void help() {
    auto branding = nc::branding();
    branding.setApplicationName("Nocode");

    qout << "Usage: " << self << " [options] [--] file..." << endl
         << endl
         << "Options:" << endl
         << "  --help, -h                  Produce this help message and quit." << endl
         << "  --verbose, -v               Print progress information to stderr." << endl
         << "  --print-sections[=FILE]     Print information about sections of the executable file." << endl
         << "  --print-symbols[=FILE]      Print the symbols from the executable file." << endl
         << "  --print-instructions[=FILE] Print parsed instructions to the file." << endl
         << "  --print-cfg[=FILE]          Print control flow graph in DOT language to the file." << endl
         << "  --print-ir[=FILE]           Print intermediate representation in DOT language to the file." << endl
         << "  --print-regions[=FILE]      Print results of structural analysis in DOT language to the file." << endl
         << "  --print-cxx[=FILE]          Print reconstructed program into given file." << endl
         << endl
         << branding.applicationName() << " is a command-line native code to C/C++ decompiler." << endl
         << "It parses given files, decompiles them, and prints the requested" << endl
         << "information (by default, C++ code) to the specified files." << endl
         << "When a file name is '-' or omitted, stdout is used." << endl
         << endl;

    qout << "Version: " << branding.applicationVersion() << endl;

    qout << "Available architectures:";
    foreach (auto architecture, nc::core::arch::ArchitectureRepository::instance()->architectures()) {
        qout << " " << architecture->name();
    }
    qout << endl;
    qout << "Available parsers:";
    foreach(auto parser, nc::core::input::ParserRepository::instance()->parsers()) {
        qout << " " << parser->name();
    }
    qout << endl;
    qout << "Report bugs to: " << branding.reportBugsTo() << endl;
    qout << "License: " << branding.licenseName() << " <" << branding.licenseUrl() << ">" << endl;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    try {
        QString sectionsFile;
        QString symbolsFile;
        QString instructionsFile;
        QString cfgFile;
        QString irFile;
        QString regionsFile;
        QString cxxFile;

        bool autoDefault = true;
        bool verbose = false;

        std::vector<nc::ByteAddr> functionAddresses;
        std::vector<nc::ByteAddr> callAddresses;

        QStringList files;

        auto args = QCoreApplication::arguments();

        for (int i = 1; i < args.size(); ++i) {
            QString arg = args[i];
            if (arg == "--help" || arg == "-h") {
                help();
                return 1;
            } else if (arg == "--verbose" || arg == "-v") {
                verbose = true;

            #define FILE_OPTION(option, variable)       \
            } else if (arg == option) {                 \
                variable = "-";                         \
                autoDefault = false;                    \
            } else if (arg.startsWith(option "=")) {    \
                variable = arg.section('=', 1);         \
                autoDefault = false;

            FILE_OPTION("--print-sections", sectionsFile)
            FILE_OPTION("--print-symbols", symbolsFile)
            FILE_OPTION("--print-instructions", instructionsFile)
            FILE_OPTION("--print-cfg", cfgFile)
            FILE_OPTION("--print-ir", irFile)
            FILE_OPTION("--print-regions", regionsFile)
            FILE_OPTION("--print-cxx", cxxFile)

            #undef FILE_OPTION

            } else if (arg == "--") {
                while (++i < args.size()) {
                    files.append(args[i]);
                }
            } else if (arg.startsWith("-")) {
                throw nc::Exception(QString("unknown argument: %1").arg(arg));
            } else {
                files.append(args[i]);
            }
        }

        if (autoDefault) {
            cxxFile = "-";
        }

        if (files.empty()) {
            throw nc::Exception("no input files");
        }

        nc::core::Context context;

        if (verbose) {
            context.setLogToken(nc::LogToken(std::make_shared<nc::StreamLogger>(qerr)));
        }

        foreach (const QString &filename, files) {
            try {
                nc::core::Driver::parse(context, filename);
            } catch (const nc::Exception &e) {
                throw nc::Exception(filename + ":" + e.unicodeWhat());
            } catch (const std::exception &e) {
                throw nc::Exception(filename + ":" + e.what());
            }
        }

        openFileForWritingAndCall(sectionsFile, [&](QTextStream &out) { printSections(context, out); });
        openFileForWritingAndCall(symbolsFile, [&](QTextStream &out) { printSymbols(context, out); });

        if (!instructionsFile.isEmpty() || !cfgFile.isEmpty() || !irFile.isEmpty() || !regionsFile.isEmpty() || !cxxFile.isEmpty()) {
            nc::core::Driver::disassemble(context);
            openFileForWritingAndCall(instructionsFile, [&](QTextStream &out) { context.instructions()->print(out); });

            if (!cfgFile.isEmpty() || !irFile.isEmpty() || !regionsFile.isEmpty() || !cxxFile.isEmpty()) {
                nc::core::Driver::decompile(context);

                openFileForWritingAndCall(cfgFile,     [&](QTextStream &out) { context.program()->print(out); });
                openFileForWritingAndCall(irFile,      [&](QTextStream &out) { context.functions()->print(out); });
                openFileForWritingAndCall(regionsFile, [&](QTextStream &out) { printRegionGraphs(context, out); });
                openFileForWritingAndCall(cxxFile,     [&](QTextStream &out) { context.tree()->print(out); });
            }
        }
    } catch (const nc::Exception &e) {
        qerr << self << ": " << e.unicodeWhat() << endl;
        return 1;
    }

    return 0;
}

/* vim:set et sts=4 sw=4: */
