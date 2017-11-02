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

#include "IdaFrontend.h"

#include "IdaWorkaroundStart.h"
#include <ida.hpp>
#include <segment.hpp>
#include <bytes.hpp>
#include <ua.hpp>
#include <funcs.hpp>  /* For get_func_name. */
#include <demangle.hpp>
#include <idp.hpp>    /* Required by intel.hpp. */
#include <intel.hpp>
#include <nalt.hpp>   /* For import_node. */
#include "IdaWorkaroundEnd.h"

#include <nc/common/CheckedCast.h>
#include <nc/common/make_unique.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>

#include "IdaByteSource.h"

namespace nc { namespace ida {
 
bool IdaFrontend::jumpToAddress(ByteAddr address) {
    return ::jumpto(checked_cast<ea_t>(address));
}

QString IdaFrontend::functionName(ByteAddr address) {
	qstring buffer;
    if(::get_func_name(&buffer, checked_cast<ea_t>(address)) == NULL)
        return QString();
    return QLatin1String(buffer.c_str());
}

QString IdaFrontend::addressName(ByteAddr address) {
    qstring buffer;
    if(::get_ea_name(&buffer, checked_cast<ea_t>(address)) == NULL)
        return QString();
    return QLatin1String(buffer.c_str());
}

int idaapi import_num_cb(ea_t ea, const char *name, uval_t ord, void *param)
{
	std::vector<std::pair<ByteAddr, QString> >& result = *(std::vector<std::pair<ByteAddr, QString> >*)param;
	result.push_back(std::make_pair(ea, QLatin1String(name)));
	return 1;
}

std::vector<std::pair<ByteAddr, QString> > IdaFrontend::importedFunctions() {
    std::vector<std::pair<ByteAddr, QString> > result;

    for(int i = 0, n = ::get_import_module_qty(); i < n; i++) {
        ::enum_import_names(i, import_num_cb, &result);
    }

    return result;
}

QString IdaFrontend::demangle(const QString &mangled) {
    QString preprocessed = mangled;
    if(preprocessed.startsWith("j_?"))
        preprocessed = preprocessed.mid(2); /* Handle thunks. */

    char buffer[MAXSTR];
    if(::demangle(buffer, sizeof(buffer), preprocessed.toLatin1().data(), 0) > 0)
        return QLatin1String(buffer);

    return preprocessed;
}

std::vector<ByteAddr> IdaFrontend::functionStarts() {
    std::vector<ByteAddr> result;

    for(std::size_t i = 0, n = get_func_qty(); i < n; i++) {
        func_t *func = getn_func(i);
        if(func != NULL)
            result.push_back(func->start_ea);
    }

    return result;
}

void IdaFrontend::createSections(core::image::Image *image) {
    for (int i = 0; i < get_segm_qty(); i++) {
        segment_t *idaSegment = getnseg(i);

        assert(idaSegment != NULL);

        qstring segName;
        ssize_t segNameSize = get_segm_name(&segName, idaSegment);
        if(segNameSize < 0) {
            segName = "";
        } else if(segNameSize > 0 && segName[0] == '_') {
            segName[0] = '.';
        }

        auto section = std::make_unique<core::image::Section>(
            segName.c_str(),
            checked_cast<ByteAddr>(idaSegment->start_ea),
            checked_cast<ByteSize>(idaSegment->size())
        );

        section->setReadable(idaSegment->perm & SEGPERM_READ);
        section->setWritable(idaSegment->perm & SEGPERM_WRITE);
        section->setExecutable(idaSegment->perm & SEGPERM_EXEC);
        section->setCode(idaSegment->type == SEG_CODE);
        section->setData(idaSegment->type == SEG_DATA);
        section->setBss(idaSegment->type == SEG_BSS);
        section->setAllocated(section->isCode() || section->isData() || section->isBss());
        section->setExternalByteSource(std::make_unique<IdaByteSource>());

        image->addSection(std::move(section));
    }
}

ByteOrder IdaFrontend::byteOrder() {
    return inf.is_be() ? ByteOrder::BigEndian : ByteOrder::LittleEndian;
}

QString IdaFrontend::architecture() {
    if (inf.procname == QLatin1String("ARM")) {
        return QLatin1String(byteOrder() == ByteOrder::LittleEndian ? "arm-le" : "arm-be");
    } else if (inf.procname == QLatin1String("ARMB")) {
        return QLatin1String("arm-be");
    } else {
        /* Assume x86 by default. */
        if (segment_t *segment = get_segm_by_name(".text")) {
            switch (segment->bitness) {
                case 0: return QLatin1String("8086");
                case 1: return QLatin1String("i386");
                case 2: return QLatin1String("x86-64");
            }
        }

        return QLatin1String("i386");
    }
}

core::image::Platform::OperatingSystem IdaFrontend::operatingSystem() {
    if (inf.filetype == f_WIN || inf.filetype == f_COFF || inf.filetype == f_PE) {
        return core::image::Platform::Windows;
    }
    return core::image::Platform::UnknownOS;
}

std::vector<AddressRange> IdaFrontend::functionAddresses(ByteAddr address) {
    std::vector<AddressRange> result;

    ea_t func_ea = checked_cast<ea_t>(address);
    func_t *function = ::get_func(func_ea);

    if (!function) {
        return result;
    }

    auto startAddr = checked_cast<ByteAddr>(function->start_ea);
    auto endAddr = checked_cast<ByteAddr>(function->end_ea);
    result.push_back(AddressRange(startAddr, endAddr));

    for (int i = 0; i < function->tailqty; ++i) {
        auto chunkStartAddr = checked_cast<ByteAddr>(function->tails[i].start_ea);
        auto chunkEndAddr = checked_cast<ByteAddr>(function->tails[i].end_ea);
        result.push_back(AddressRange(chunkStartAddr, chunkEndAddr));
    }
    
    return result;
}

ByteAddr IdaFrontend::screenAddress() {
    return checked_cast<ByteAddr>(get_screen_ea());
}

void IdaFrontend::addMenuItem(const QString &menuItem, const action_desc_t& actionDesc) {
	::register_action(actionDesc);
	::attach_action_to_menu(
		menuItem.toLocal8Bit().constData(),
		actionDesc.name,
		SETMENU_APP);
}

void IdaFrontend::deleteMenuItem(const QString &menuItem, const action_desc_t& actionDesc) {
	::unregister_action(actionDesc.name);
	::detach_action_from_menu(
		menuItem.toLocal8Bit().constData(),
		actionDesc.name);
}

void IdaFrontend::print(const QString &message) {
    ::msg(message.toLocal8Bit());
}

QWidget *IdaFrontend::createWidget(const QString &caption) {
    TWidget *widget = find_widget(caption.toLocal8Bit().constData());
    display_widget(widget, FORM_MDI | FORM_TAB | FORM_MENU | FORM_QWIDGET);
    return reinterpret_cast<QWidget *>(widget);
}

void IdaFrontend::activateWidget(QWidget *widget) {
    ::activate_widget(reinterpret_cast<TWidget *>(widget), true);
}

void IdaFrontend::closeWidget(QWidget *widget) {
    ::close_widget(reinterpret_cast<TWidget *>(widget), 0);
}

}} // namespace nc::ida

/* vim:set et sts=4 sw=4: */
