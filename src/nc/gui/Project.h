/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <QObject>

#include <cassert>
#include <memory>
#include <vector>

#include <nc/common/Types.h>
#include <nc/common/LogToken.h>

namespace nc {

namespace core {
    class Context;

    namespace arch {
        class Instruction;
        class Instructions;
    }

    namespace image {
        class ByteSource;
        class Image;
    }
}

namespace gui {

class CommandQueue;
class Decompile;

/**
 * Class providing high-level model of the decompilation project.
 */
class Project: public QObject {
    Q_OBJECT

    /** Name of the project. */
    QString name_;

    /** Executable image being decompiled. */
    std::shared_ptr<core::image::Image> image_;

    /** Disassembled instructions. */
    std::shared_ptr<const core::arch::Instructions> instructions_;

    /** Current context. */
    std::shared_ptr<const core::Context> context_;

    /** Log token. */
    LogToken logToken_;

    /** Queue of user commands. */
    CommandQueue *commandQueue_;

    public:

    /**
     * Constructor.
     *
     * Creates an empty project with empty context.
     *
     * \param parent Pointer to the parent object. Can be nullptr.
     */
    Project(QObject *parent = nullptr);

    /**
     * Destructor.
     */
    ~Project();

    /**
     * \return Name of the project.
     */
    const QString &name() const { return name_; }

    /**
     * Sets the name of the project.
     */
    void setName(const QString &name);

    /**
     * \return Valid pointer to the executable image being decompiled.
     */
    std::shared_ptr<core::image::Image> image() const { return image_; }

    /**
     * Sets the executable image being decompiled.
     *
     * \param image Valid pointer to the image.
     */
    void setImage(const std::shared_ptr<core::image::Image> &image);

    /**
     * \returns Valid pointer to the instructions of the executable file.
     */
    const std::shared_ptr<const core::arch::Instructions> &instructions() const { return instructions_; }

    /**
     * Sets the set instructions of the executable file.
     *
     * \param instructions New set of instructions.
     */
    void setInstructions(const std::shared_ptr<const core::arch::Instructions> &instructions);

    /**
     * \return Pointer to the current context instance. Can be nullptr.
     */
    const std::shared_ptr<const core::Context> &context() const { assert(context_); return context_; }

    /**
     * Sets current context.
     *
     * \param context Valid pointer to the new context.
     */
    void setContext(const std::shared_ptr<const core::Context> &context);

    /**
     * Sets the log token.
     *
     * \param token log token.
     */
    void setLogToken(const LogToken &token) { logToken_ = token; }

    /**
     * \return Log token.
     */
    const LogToken &logToken() const { return logToken_; }

    /*
     * \return Valid pointer to command queue.
     */
    CommandQueue *commandQueue() const { return commandQueue_; }

    /**
     * Schedules deletion of given instructions.
     *
     * \param instructions List of instructions to delete.
     */
    void deleteInstructions(const std::vector<const core::arch::Instruction *> &instructions);

    /**
     * Schedules disassembly of all instructions in the given range of addresses.
     *
     * \param source Valid pointer to a byte source.
     * \param begin First address in the range.
     * \param end First address past the range.
     */
    void disassemble(const core::image::ByteSource *source, ByteAddr begin, ByteAddr end);

    /**
     * Schedules decompilation of the given set of instructions.
     *
     * \param instructions Instructions to be decompiled.
     */
    void decompile(const std::vector<const core::arch::Instruction *> &instructions);

    /**
     * Schedules decompilation of the given set of instructions.
     *
     * \param instructions Valid pointer to the instructions to be decompiled.
     */
    void decompile(const std::shared_ptr<const core::arch::Instructions> &instructions);

    public Q_SLOTS:

    /**
     * Schedules disassembly of all code sections.
     */
    void disassemble();

    /**
     * Schedules decompilation of all the instructions of the project.
     */
    void decompile();

    /**
     * Cancels all scheduled commands.
     */
    void cancelAll();

    Q_SIGNALS:

    /**
     * Signal emitted when the project gets a new name.
     */
    void nameChanged();

    /**
     * Signal emitted when a new image has been set.
     */
    void imageChanged();

    /**
     * Signal emitted when the set of instructions is changed.
     */
    void instructionsChanged();

    /**
     * Signal emitted when C tree is computed.
     */
    void treeChanged();

    private Q_SLOTS:

    /**
     * Takes and sets the set of instructions from context.
     */
    void updateInstructions();
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
