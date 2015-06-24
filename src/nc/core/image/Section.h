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

#include <memory> /* std::unique_ptr */

#include <QByteArray>
#include <QString>

#include <nc/common/Types.h>

#include "ByteSource.h"

namespace nc {
namespace core {
namespace image {

/**
 * Section of an executable file.
 */
class Section: public ByteSource {
    QString name_; ///< Name of the section.

    ByteAddr addr_; ///< Linear address of section start.
    ByteSize size_; ///< Size of the section.

    bool isAllocated_; ///< True if the section is mapped to the memory when the program is executed.

    bool isReadable_; ///< True if the section is readable.
    bool isWritable_; ///< True if the section is writable.
    bool isExecutable_; ///< True if the section is executable.

    bool isCode_; ///< True if the section contains code.
    bool isData_; ///< True if the section contains data.
    bool isBss_; ///< True if the section is bss.

    QByteArray content_; ///< Data contained in the section.
    std::unique_ptr<ByteSource> externalByteSource_; ///< External source of this section's bytes.

public:
    /**
     * Constructor.
     *
     * \param[in] name  Name of the section.
     * \param[in] addr  Linear address of the section's start.
     * \param[in] size  Size of the section.
     */
    Section(const QString &name, ByteAddr addr, ByteSize size);

    /**
     * \return Name of the section.
     */
    const QString &name() const { return name_; }

    /**
     * Sets name of the section.
     *
     * \param name New name of the section.
     */
    void setName(const QString &name) { name_ = name; }

    /**
     * \return Virtual address of the section's start.
     */
    ByteAddr addr() const { return addr_; }

    /**
     * Sets a new virtual address of the section's start.
     *
     * \param[in] addr New address.
     */
    void setAddr(ByteAddr addr) { addr_ = addr; }

    /**
     * \return Size of the section.
     */
    ByteSize size() const { return size_; }

    /**
     * Sets the size of the section.
     *
     * \param[in] size New size of the section.
     */
    void setSize(ByteSize size) { size_ = size; }

    /**
     * \return Virtual address of the section's end.
     */
    ByteAddr endAddr() const { return addr_ + size_; }

    /**
     * \param[in] addr Virtual address.
     *
     * \return True if this section contains given virtual address.
     */
    bool containsAddress(ByteAddr addr) const { return addr_ <= addr && addr < addr_ + size_; }

    /**
     * \return True if the section occupied memory.
     */
    bool isAllocated() const { return isAllocated_; }

    /**
     * Sets whether the section is allocated.
     *
     * \param[in] isAllocated Whether the section is mapped to the memory when the program is executed.
     */
    void setAllocated(bool isAllocated = true) { isAllocated_ = isAllocated; }

    /**
     * \return True if the section is readable.
     */
    bool isReadable() const { return isReadable_; }

    /**
     * Sets whether the section is readable.
     *
     * \param[in] isReadable Whether the section is readable.
     */
    void setReadable(bool isReadable = true) { isReadable_ = isReadable; }

    /**
     * \return True if the section is writable.
     */
    bool isWritable() const { return isWritable_; }

    /**
     * Sets whether the section is writable.
     *
     * \param[in] isWritable Whether the section is writable.
     */
    void setWritable(bool isWritable = true) { isWritable_ = isWritable; }

    /**
     * \return True if the section is writable.
     */
    bool isExecutable() const { return isExecutable_; }

    /**
     * Sets whether the section is executable.
     *
     * \param[in] isExecutable Whether the section is executable.
     */
    void setExecutable(bool isExecutable = true) { isExecutable_ = isExecutable; }

    /**
     * \return True if the section contains code.
     */
    bool isCode() const { return isCode_; }

    /**
     * Sets whether the section contains code.
     *
     * \param[in] isCode Whether section contains code.
     */
    void setCode(bool isCode = true) { isCode_ = isCode; }

    /**
     * Sets whether the section contains data.
     */
    bool isData() const { return isData_; }

    /**
     * Sets whether the section contains data.
     *
     * \param[in] isData Whether section contains code.
     */
    void setData(bool isData = true) { isData_ = isData; }

    /**
     * \return True if the section contains no data.
     */
    bool isBss() const { return isBss_; }

    /**
     * Sets whether the section contains no data).
     */
    void setBss(bool isBss = true) { isBss_ = isBss; }

    /**
     * \return Pointer to the external byte source. Can be nullptr.
     */
    ByteSource *externalByteSource() const { return externalByteSource_.get(); }

    /**
     * Sets the content of the section.
     *
     * \param content New content.
     */
    void setContent(QByteArray content) { content_ = std::move(content); }

    /**
     * Sets the external byte source with the content of the section.
     *
     * \param byteSource Pointer to the new external byte source. Can be nullptr.
     */
    void setExternalByteSource(std::unique_ptr<ByteSource> byteSource) { externalByteSource_ = std::move(byteSource); }

    /**
     * Reads a sequence of bytes from the section.
     * Delegates the reading to the external byte source, if set.
     * If not set, reads from the QByteArray given to setContent().
     * If the array is smaller than the section, reading from beyond the array yields zeroes.
     */
    ByteSize readBytes(ByteAddr addr, void *buf, ByteSize size) const override;
};

} // namespace image
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
