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

#include <QTextStream>

namespace nc {

/**
 * Base class for printable objects. The objects must have
 * void print(QTextStream &) const method which will be called
 * by the respective << operator.
 *
 * \tparam T Derived class.
 */
template<class T>
class PrintableBase {
    public:

    /**
     * Prints the object into a stream.
     *
     * \param out Output stream.
     */
    void print(QTextStream &out) const {
        static_cast<const T *>(this)->print(out);
    }

    /**
     * \return String representation of the object.
     */
    QString toString() const {
        QString result;
        QTextStream stream(&result);

        print(stream);

        return result;
    }
};

/**
 * Base class for polymorphic printable objects.
 *
 * Consider inheriting from PrintableBase for non-polymorphic classes.
 */
class Printable: public PrintableBase<Printable> {
    public:

    /**
     * Prints the object into a stream.
     *
     * \param out Output stream.
     */
    virtual void print(QTextStream &out) const = 0;

    /**
     * Virtual destructor.
     */
    virtual ~Printable() {}
};

/**
 * Insertion operator for printable objects.
 *
 * \param out Output stream.
 * \param[in] entity Object to be printed.
 *
 * \return The output stream.
 */
template<class T>
inline QTextStream &operator<<(QTextStream &out, const PrintableBase<T> &entity) {
    entity.print(out);
    return out;
}

} // namespace nc

/* vim:set et sts=4 sw=4: */
