/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QCoreApplication>
#include <QString>

#include <nc/common/Types.h>

namespace nc {
namespace core {

namespace image {
    class Image;
    class Symbol;
}

namespace ir {

class Function;
class MemoryLocation;
class Term;

namespace calling {
    class CalleeId;
}

namespace cgen {

class NameAndComment {
    QString name_;
    QString comment_;

public:
    NameAndComment(QString name = QString(), QString comment = QString()):
        name_(std::move(name)), comment_(std::move(comment))
    {}

    QString &name() { return name_; }
    const QString &name() const { return name_; }

    QString &comment() { return comment_; }
    const QString &comment() const { return comment_; }

    operator const void*() const { return name_.isEmpty() ? nullptr : this; }
};

class NameGenerator {
    Q_DECLARE_TR_FUNCTIONS(NameGenerator)

    const image::Image &image_;
public:
    NameGenerator(const image::Image &image): image_(image) {}

    NameAndComment getFunctionName(const calling::CalleeId &calleeId) const;
    NameAndComment getFunctionName(const Function *function) const;
    NameAndComment getFunctionName(ByteAddr addr) const;
    NameAndComment getFunctionName(const image::Symbol *symbol) const;

    NameAndComment getGlobalVariableName(const MemoryLocation &memoryLocation) const;
    NameAndComment getGlobalVariableName(ByteAddr addr) const;
    NameAndComment getGlobalVariableName(const image::Symbol *symbol) const;

    NameAndComment getLocalVariableName(const MemoryLocation &memoryLocation, std::size_t serial) const;

    NameAndComment getArgumentName(const Term *term, std::size_t serial) const;

    static QString cleanName(const QString &name);
};

}}}} // namespace nc::core::ir::cgen

/* vim:set et sts=4 sw=4: */
