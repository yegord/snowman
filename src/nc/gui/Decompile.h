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

#include <nc/config.h>

#include <memory>

#include "Command.h"

namespace nc {

namespace core {
    namespace arch {
        class Instructions;
    }
}

namespace gui {

class Project;

/**
 * 'Decompile' command.
 */
class Decompile: public Command {
    Q_OBJECT

    /** Project. */
    Project *project_;

    /** Instructions to be decompiled. */
    std::shared_ptr<const core::arch::Instructions> instructions_;

    public:

    /**
     * Constructor.
     *
     * \param project Valid pointer to a project.
     * \param instructions List of instructions to be decompiled.
     */
    Decompile(Project *project, const std::shared_ptr<const core::arch::Instructions> &instructions);

    protected:

    void work() override;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
