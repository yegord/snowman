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

#include <memory>

#include <boost/optional.hpp>

#include <QDialog>

#include <nc/common/Types.h>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLineEdit;
QT_END_NAMESPACE

namespace nc {

namespace core {
    namespace image {
        class Section;
        class Image;
    }
}

namespace gui {

/**
 * Dialog letting to choose a section and a range of addresses to disassemble.
 */
class DisassemblyDialog: public QDialog {
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * \param parent Parent widget.
     */
    explicit DisassemblyDialog(QWidget *parent = nullptr);

    /**
     * Sets the associated executable image.
     *
     * \param image Pointer to the image. Can be nullptr.
     */
    void setImage(const std::shared_ptr<const core::image::Image> &image = std::shared_ptr<const core::image::Image>());

    /**
     * \return Pointer to the executable image. Can be nullptr.
     */
    const std::shared_ptr<const core::image::Image> &image() const { return image_; }

    /**
     * \return Pointer to the currently selected section. Can be nullptr.
     */
    const core::image::Section *selectedSection() const;

    /**
     * Selects given section.
     *
     * \param section Valid pointer to a section.
     */
    void selectSection(const core::image::Section *section);

    /**
     * \return Chosen start address, or boost::none, if not specified.
     */
    boost::optional<ByteAddr> startAddress() const;

    /**
     * \return Chosen end address, or boost::none, if not specified.
     */
    boost::optional<ByteAddr> endAddress() const;

    public Q_SLOTS:

    /**
     * Updates the list of sections to choose from.
     */
    void updateSectionsList();

    /**
     * Fills in start and end addresses depending on selected section.
     */
    void updateAddresses();

    void accept() override;

private:
    /** Associated executable image. */
    std::shared_ptr<const core::image::Image> image_;

    /** Combo box for choosing the section. */
    QComboBox *sectionComboBox_;

    /** Input for the start address. */
    QLineEdit *startLineEdit_;

    /** Input for the end address. */
    QLineEdit *endLineEdit_;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
