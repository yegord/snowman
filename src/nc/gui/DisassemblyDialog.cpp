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

#include "DisassemblyDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>

#include <nc/common/Foreach.h>
#include <nc/common/StringToInt.h>

#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>

Q_DECLARE_METATYPE(const nc::core::image::Section *)

namespace nc {
namespace gui {

DisassemblyDialog::DisassemblyDialog(QWidget *parent):
    QDialog(parent, Qt::Dialog)
{
    setWindowTitle(tr("Disassemble"));

    sectionComboBox_ = new QComboBox(this);

    QRegExpValidator *hexValidator = new QRegExpValidator(QRegExp("[0123456789abcdef]+"), this);

    startLineEdit_ = new QLineEdit(this);
    startLineEdit_->setValidator(hexValidator);

    endLineEdit_ = new QLineEdit(this);
    endLineEdit_->setValidator(hexValidator);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QFormLayout *layout = new QFormLayout(this);
    layout->addRow(tr("Section:"), sectionComboBox_);
    layout->addRow(tr("Start address:"), startLineEdit_);
    layout->addRow(tr("End address:"), endLineEdit_);
    layout->addRow(buttonBox);
    setLayout(layout);

    connect(sectionComboBox_, SIGNAL(activated(int)), this, SLOT(updateAddresses()));

    updateSectionsList();
}

void DisassemblyDialog::setImage(const std::shared_ptr<const core::image::Image> &image) {
    if (image != image_) {
        image_ = image;
        updateSectionsList();
        updateAddresses();
    }
}

void DisassemblyDialog::updateSectionsList() {
    sectionComboBox_->clear();

    if (image()) {
        foreach (auto section, image()->sections()) {
            sectionComboBox_->addItem(section->name(), QVariant::fromValue(section));
        }
    }
}

void DisassemblyDialog::updateAddresses() {
    if (const core::image::Section *section = selectedSection()) {
        startLineEdit_->setText(QString("%1").arg(section->addr(), 0, 16));
        endLineEdit_->setText(QString("%1").arg(section->endAddr(), 0, 16));
    } else {
        startLineEdit_->clear();
        endLineEdit_->clear();
    }
}

const core::image::Section *DisassemblyDialog::selectedSection() const {
    return sectionComboBox_->itemData(sectionComboBox_->currentIndex()).value<const core::image::Section *>();
}

void DisassemblyDialog::selectSection(const core::image::Section *section) {
    sectionComboBox_->setCurrentIndex(sectionComboBox_->findData(QVariant::fromValue(section)));
    updateAddresses();
}

boost::optional<ByteAddr> DisassemblyDialog::startAddress() const {
    return stringToInt<ByteAddr>(startLineEdit_->text(), 16);
}

boost::optional<ByteAddr> DisassemblyDialog::endAddress() const {
    return stringToInt<ByteAddr>(endLineEdit_->text(), 16);
}

void DisassemblyDialog::accept() {
    auto section = selectedSection();
    if (!section) {
        QMessageBox::critical(this, tr("Error"), tr("Please, select a section."));
        return;
    }

    auto startAddr = startAddress();
    if (!startAddr || *startAddr < section->addr() || *startAddr > section->endAddr()) {
        QMessageBox::critical(this, tr("Error"), tr("Please, specify the start address within the section's address range."));
        return;
    }

    auto endAddr = endAddress();
    if (!endAddr || *endAddr < section->addr() || *endAddr > section->endAddr()) {
        QMessageBox::critical(this, tr("Error"), tr("Please, specify the end address within the section's address range."));
        return;
    }

    if (*startAddr >= *endAddr) {
        QMessageBox::critical(this, tr("Error"), tr("Please, specify a non-empty range of addresses."));
        return;
    }

    QDialog::accept();
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
