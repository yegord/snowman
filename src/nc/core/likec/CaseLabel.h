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

#include <cassert>
#include <memory>

#include "Statement.h"

namespace nc {
namespace core {
namespace likec {

class Expression;

/**
 * Case label.
 */
class CaseLabel: public Statement {
    /** Case expression. */
    std::unique_ptr<Expression> expression_;

public:
    /**
     * Class constructor.
     *
     * \param[in] expression Valid pointer to the case expression.
     */
    explicit CaseLabel(std::unique_ptr<Expression> expression):
        Statement(CASE_LABEL), expression_(std::move(expression))
    {
        assert(expression_);
    }

    /**
     * \return Valid pointer to the case expression.
     */
    Expression *expression() const { return expression_.get(); }

protected:
    void doPrint(PrintContext &callback) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Statement, nc::core::likec::CaseLabel, nc::core::likec::Statement::CASE_LABEL)

/* vim:set et sts=4 sw=4: */
