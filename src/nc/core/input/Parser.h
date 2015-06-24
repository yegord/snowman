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
#include <QString>

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE

namespace nc {

class LogToken;

namespace core {

class Instructions;

namespace image {
    class Image;
}

namespace input {

/**
 * Input parser.
 * 
 * Note that this interface does not define the 'real' parser. 
 * It defines a factory-like object that can be used to:
 * <ul>
 * <li>Create a 'real' parser and parse the given source using it.</li>
 * <li>Check whether the given source seems to be parsable by this parser.</li>
 * </ul>
 * 
 * This design was chosen over the classic abstract factory because there
 * are no practical advantages in separating the 'real' parser into an interface.
 * 
 * Input parser is expected to be stateless and can be accessed by multiple 
 * threads. In general, single instance of a concrete input parser should be
 * enough for the whole application.
 */
class Parser: public QObject {
    QString name_; ///< Name of this parser.

public:
    /**
     * Constructor.
     * 
     * \param[in] name Name of this parser.
     */
    Parser(QString name): name_(std::move(name)) {}

    /**
     * Virtual destructor.
     */
    virtual ~Parser() {}

    /**
     * \returns Name of this parser.
     */
    const QString &name() const { return name_; }

    /**
     * \param[in] source Valid pointer to the data source.
     *
     * \returns Whether the data at source looks like
     *          something that can be parsed with this parser.
     */
    bool canParse(QIODevice *source) const;

    /**
     * Parse executable image from the given IO device.
     *
     * \param[in] source Valid pointer to the data source.
     * \param[out] image Valid pointer to the image.
     * \param[in] log Log token.
     */
    void parse(QIODevice *source, image::Image *image, const LogToken &log) const;

protected:
    /**
     * \param[in] source Data source.
     *
     * \returns Whether the data at source looks like
     *          something that can be parsed with this parser.
     */
    virtual bool doCanParse(QIODevice *source) const = 0;

    /**
     * Actually parses executable image from the given IO device.
     *
     * \param[in] source Data source.
     * \param[out] image Valid pointer to the image.
     * \param[in] log Log token.
     */
    virtual void doParse(QIODevice *source, image::Image *image, const LogToken &log) const = 0;
};

}}} // namespace nc::core::input

/* vim:set et sts=4 sw=4: */
