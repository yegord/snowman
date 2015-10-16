## Process this file with automake to generate Makefile.in
#
#   Copyright (C) 2012-2014 Free Software Foundation, Inc.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING3.  If not see
# <http://www.gnu.org/licenses/>.
#

AUTOMAKE_OPTIONS = 1.9 cygnus

DOCFILES = aoutx.texi  archive.texi archures.texi \
	bfdt.texi  cache.texi coffcode.texi \
	core.texi elf.texi elfcode.texi  format.texi \
	libbfd.texi bfdwin.texi bfdio.texi \
	opncls.texi  reloc.texi  section.texi  \
	syms.texi  targets.texi init.texi hash.texi linker.texi \
	mmo.texi \
	bfdver.texi

PROTOS = archive.p archures.p bfd.p \
	 core.p format.p \
	bfdio.p bfdwin.p \
	libbfd.p opncls.p reloc.p \
	section.p syms.p targets.p  \
	format.p  core.p init.p

IPROTOS = cache.ip libbfd.ip reloc.ip init.ip archures.ip coffcode.ip

# SRCDOC, SRCPROT, SRCIPROT only used to sidestep Sun Make bug in interaction 
# between VPATH and suffix rules.  If you use GNU Make, perhaps other Makes,
# you don't need these three:
SRCDOC = $(srcdir)/../aoutx.h  $(srcdir)/../archive.c \
	$(srcdir)/../archures.c $(srcdir)/../bfd.c \
	$(srcdir)/../bfdio.c $(srcdir)/../bfdwin.c \
	$(srcdir)/../cache.c $(srcdir)/../coffcode.h \
	$(srcdir)/../corefile.c $(srcdir)/../elf.c \
	$(srcdir)/../elfcode.h  $(srcdir)/../format.c \
	$(srcdir)/../libbfd.c $(srcdir)/../opncls.c \
	$(srcdir)/../reloc.c  $(srcdir)/../section.c \
	$(srcdir)/../syms.c  $(srcdir)/../targets.c \
	$(srcdir)/../hash.c $(srcdir)/../linker.c \
	$(srcdir)/../mmo.c

SRCPROT = $(srcdir)/../archive.c $(srcdir)/../archures.c \
	$(srcdir)/../bfd.c $(srcdir)/../coffcode.h $(srcdir)/../corefile.c \
	$(srcdir)/../format.c $(srcdir)/../libbfd.c \
	$(srcdir)/../bfdio.c $(srcdir)/../bfdwin.c \
	$(srcdir)/../opncls.c $(srcdir)/../reloc.c \
	$(srcdir)/../section.c $(srcdir)/../syms.c \
	$(srcdir)/../targets.c $(srcdir)/../init.c

SRCIPROT = $(srcdir)/../cache.c $(srcdir)/../libbfd.c \
	$(srcdir)/../bfdio.c $(srcdir)/../bfdwin.c \
	$(srcdir)/../reloc.c $(srcdir)/../cpu-h8300.c \
	$(srcdir)/../cpu-i960.c $(srcdir)/../archures.c \
	$(srcdir)/../init.c

TEXIDIR = $(srcdir)/../../texinfo/fsf

info_TEXINFOS = bfd.texinfo
bfd_TEXINFOS = $(DOCFILES) bfdsumm.texi

MKDOC = chew$(EXEEXT_FOR_BUILD)

AM_CPPFLAGS = -I.. -I$(srcdir)/.. -I$(srcdir)/../../include \
	-I$(srcdir)/../../intl -I../../intl

$(MKDOC): chew.stamp ; @true
chew.stamp: $(srcdir)/chew.c
	$(CC_FOR_BUILD) -o chw$$$$$(EXEEXT_FOR_BUILD) $(CFLAGS_FOR_BUILD) \
	  $(LDFLAGS_FOR_BUILD) $(H_CFLAGS) $(AM_CPPFLAGS) $(srcdir)/chew.c; \
	$(SHELL) $(srcdir)/../../move-if-change \
	  chw$$$$$(EXEEXT_FOR_BUILD) $(MKDOC); \
	touch $@

protos: libbfd.h libcoff.h bfd.h

# We can't replace these rules with an implicit rule, because
# makes without VPATH support couldn't find the .h files in `..'.

# We do not depend on chew directly so that we can distribute the info
# files, and permit people to rebuild them, without requiring the makeinfo
# program.  If somebody tries to rebuild info, but none of the .texi files
# have changed, then nothing will be rebuilt.

aoutx.texi: aoutx.stamp ; @true
aoutx.stamp: $(srcdir)/../aoutx.h $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../aoutx.h >aoutx.tmp
	test -e aoutx.texi || test ! -f $(srcdir)/aoutx.texi || cp -p $(srcdir)/aoutx.texi .
	$(SHELL) $(srcdir)/../../move-if-change aoutx.tmp aoutx.texi
	touch $@

archive.texi: archive.stamp ; @true
archive.stamp: $(srcdir)/../archive.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../archive.c >archive.tmp
	test -e archive.texi || test ! -f $(srcdir)/archive.texi || cp -p $(srcdir)/archive.texi .
	$(SHELL) $(srcdir)/../../move-if-change archive.tmp archive.texi
	touch $@

archures.texi: archures.stamp ; @true
archures.stamp: $(srcdir)/../archures.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../archures.c >archures.tmp
	test -e archures.texi || test ! -f $(srcdir)/archures.texi || cp -p $(srcdir)/archures.texi .
	$(SHELL) $(srcdir)/../../move-if-change archures.tmp archures.texi
	touch $@

# We use bfdt.texi, rather than bfd.texi, to avoid conflicting with
# bfd.texinfo on an 8.3 filesystem.
bfdt.texi: bfdt.stamp ; @true
bfdt.stamp: $(srcdir)/../bfd.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../bfd.c >bfd.tmp
	test -e bfdt.texi || test ! -f $(srcdir)/bfdt.texi || cp -p $(srcdir)/bfdt.texi .
	$(SHELL) $(srcdir)/../../move-if-change bfd.tmp bfdt.texi
	touch $@

cache.texi: cache.stamp ; @true
cache.stamp: $(srcdir)/../cache.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../cache.c >cache.tmp
	test -e cache.texi || test ! -f $(srcdir)/cache.texi || cp -p $(srcdir)/cache.texi .
	$(SHELL) $(srcdir)/../../move-if-change cache.tmp cache.texi
	touch $@

coffcode.texi: coffcode.stamp ; @true
coffcode.stamp: $(srcdir)/../coffcode.h $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../coffcode.h >coffcode.tmp
	test -e coffcode.texi || test ! -f $(srcdir)/coffcode.texi || cp -p $(srcdir)/coffcode.texi .
	$(SHELL) $(srcdir)/../../move-if-change coffcode.tmp coffcode.texi
	touch $@

core.texi: core.stamp ; @true
core.stamp: $(srcdir)/../corefile.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../corefile.c >core.tmp
	test -e core.texi || test ! -f $(srcdir)/core.texi || cp -p $(srcdir)/core.texi .
	$(SHELL) $(srcdir)/../../move-if-change core.tmp core.texi
	touch $@

elf.texi: elf.stamp ; @true
elf.stamp: $(srcdir)/../elf.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../elf.c >elf.tmp
	test -e elf.texi || test ! -f $(srcdir)/elf.texi || cp -p $(srcdir)/elf.texi .
	$(SHELL) $(srcdir)/../../move-if-change elf.tmp elf.texi
	touch $@

elfcode.texi: elfcode.stamp ; @true
elfcode.stamp: $(srcdir)/../elfcode.h $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../elfcode.h >elfcode.tmp
	test -e elfcode.texi || test ! -f $(srcdir)/elfcode.texi || cp -p $(srcdir)/elfcode.texi .
	$(SHELL) $(srcdir)/../../move-if-change elfcode.tmp elfcode.texi
	touch $@

mmo.texi: mmo.stamp ; @true
mmo.stamp: $(srcdir)/../mmo.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../mmo.c >mmo.tmp
	test -e mmo.texi || test ! -f $(srcdir)/mmo.texi || cp -p $(srcdir)/mmo.texi .
	$(SHELL) $(srcdir)/../../move-if-change mmo.tmp mmo.texi
	touch $@

format.texi: format.stamp ; @true
format.stamp: $(srcdir)/../format.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../format.c >format.tmp
	test -e format.texi || test ! -f $(srcdir)/format.texi || cp -p $(srcdir)/format.texi .
	$(SHELL) $(srcdir)/../../move-if-change format.tmp format.texi
	touch $@

libbfd.texi: libbfd.stamp ; @true
libbfd.stamp: $(srcdir)/../libbfd.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../libbfd.c >libbfd.tmp
	test -e libbfd.texi || test ! -f $(srcdir)/libbfd.texi || cp -p $(srcdir)/libbfd.texi .
	$(SHELL) $(srcdir)/../../move-if-change libbfd.tmp libbfd.texi
	touch $@

bfdio.texi: bfdio.stamp ; @true
bfdio.stamp: $(srcdir)/../bfdio.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../bfdio.c >bfdio.tmp
	test -e bfdio.texi || test ! -f $(srcdir)/bfdio.texi || cp -p $(srcdir)/bfdio.texi .
	$(SHELL) $(srcdir)/../../move-if-change bfdio.tmp bfdio.texi
	touch $@

bfdwin.texi: bfdwin.stamp ; @true
bfdwin.stamp: $(srcdir)/../bfdwin.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../bfdwin.c >bfdwin.tmp
	test -e bfdwin.texi || test ! -f $(srcdir)/bfdwin.texi || cp -p $(srcdir)/bfdwin.texi .
	$(SHELL) $(srcdir)/../../move-if-change bfdwin.tmp bfdwin.texi
	touch $@

opncls.texi: opncls.stamp ; @true
opncls.stamp: $(srcdir)/../opncls.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../opncls.c >opncls.tmp
	test -e opncls.texi || test ! -f $(srcdir)/opncls.texi || cp -p $(srcdir)/opncls.texi .
	$(SHELL) $(srcdir)/../../move-if-change opncls.tmp opncls.texi
	touch $@

reloc.texi: reloc.stamp ; @true
reloc.stamp: $(srcdir)/../reloc.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../reloc.c >reloc.tmp
	test -e reloc.texi || test ! -f $(srcdir)/reloc.texi || cp -p $(srcdir)/reloc.texi .
	$(SHELL) $(srcdir)/../../move-if-change reloc.tmp reloc.texi
	touch $@

section.texi: section.stamp ; @true
section.stamp: $(srcdir)/../section.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../section.c >section.tmp
	test -e section.texi || test ! -f $(srcdir)/section.texi || cp -p $(srcdir)/section.texi .
	$(SHELL) $(srcdir)/../../move-if-change section.tmp section.texi
	touch $@

syms.texi: syms.stamp ; @true
syms.stamp: $(srcdir)/../syms.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../syms.c >syms.tmp
	test -e syms.texi || test ! -f $(srcdir)/syms.texi || cp -p $(srcdir)/syms.texi .
	$(SHELL) $(srcdir)/../../move-if-change syms.tmp syms.texi
	touch $@

targets.texi: targets.stamp ; @true
targets.stamp: $(srcdir)/../targets.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../targets.c >targets.tmp
	test -e targets.texi || test ! -f $(srcdir)/targets.texi || cp -p $(srcdir)/targets.texi .
	$(SHELL) $(srcdir)/../../move-if-change targets.tmp targets.texi
	touch $@

init.texi: init.stamp ; @true
init.stamp: $(srcdir)/../init.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../init.c >init.tmp
	test -e init.texi || test ! -f $(srcdir)/init.texi || cp -p $(srcdir)/init.texi .
	$(SHELL) $(srcdir)/../../move-if-change init.tmp init.texi
	touch $@

hash.texi: hash.stamp ; @true
hash.stamp: $(srcdir)/../hash.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../hash.c >hash.tmp
	test -e hash.texi || test ! -f $(srcdir)/hash.texi || cp -p $(srcdir)/hash.texi .
	$(SHELL) $(srcdir)/../../move-if-change hash.tmp hash.texi
	touch $@

linker.texi: linker.stamp ; @true
linker.stamp: $(srcdir)/../linker.c $(srcdir)/doc.str $(MKDOC)
	./$(MKDOC) -f $(srcdir)/doc.str < $(srcdir)/../linker.c >linker.tmp
	test -e linker.texi || test ! -f $(srcdir)/linker.texi || cp -p $(srcdir)/linker.texi .
	$(SHELL) $(srcdir)/../../move-if-change linker.tmp linker.texi
	touch $@

LIBBFD_H_DEP = \
	$(srcdir)/../libbfd-in.h	\
	$(srcdir)/../init.c		\
	$(srcdir)/../libbfd.c		\
	$(srcdir)/../bfdio.c		\
	$(srcdir)/../bfdwin.c		\
	$(srcdir)/../cache.c		\
	$(srcdir)/../reloc.c		\
	$(srcdir)/../archures.c		\
	$(srcdir)/../elf.c		\
	$(srcdir)/header.sed		\
	$(srcdir)/proto.str		\
	$(MKDOC)

libbfd.h: $(LIBBFD_H_DEP)
	echo "$(LIBBFD_H_DEP)" | sed -f $(srcdir)/header.sed > $@
	for file in $(LIBBFD_H_DEP); do \
	  case $$file in \
	    *-in.h) cat $$file >> $@ ;; \
	    */header.sed) break ;; \
	    *)	echo $$file | sed -e 's,.*/,,' -e 's,^,/* Extracted from ,' \
				-e 's,$$,.  */,' >> $@ ; \
		./$(MKDOC) -i -f $(srcdir)/proto.str < $$file >> $@ ;; \
	  esac; \
	done

LIBCOFF_H_DEP = \
	$(srcdir)/../libcoff-in.h	\
	$(srcdir)/../coffcode.h		\
	$(srcdir)/header.sed		\
	$(srcdir)/proto.str		\
	$(MKDOC)

libcoff.h: $(LIBCOFF_H_DEP)
	echo "$(LIBCOFF_H_DEP)" | sed -f $(srcdir)/header.sed > $@
	for file in $(LIBCOFF_H_DEP); do \
	  case $$file in \
	    *-in.h) cat $$file >> $@ ;; \
	    */header.sed) break ;; \
	    *)	echo $$file | sed -e 's,.*/,,' -e 's,^,/* Extracted from ,' \
				-e 's,$$,.  */,' >> $@ ; \
		./$(MKDOC) -i -f $(srcdir)/proto.str < $$file >> $@ ;; \
	  esac; \
	done

BFD_H_DEP = \
	$(srcdir)/../bfd-in.h 		\
	$(srcdir)/../init.c 		\
	$(srcdir)/../opncls.c 		\
	$(srcdir)/../libbfd.c 		\
	$(srcdir)/../bfdio.c		\
	$(srcdir)/../bfdwin.c		\
	$(srcdir)/../section.c 		\
	$(srcdir)/../archures.c		\
	$(srcdir)/../reloc.c		\
	$(srcdir)/../syms.c		\
	$(srcdir)/../bfd.c		\
	$(srcdir)/../archive.c		\
	$(srcdir)/../corefile.c		\
	$(srcdir)/../targets.c		\
	$(srcdir)/../format.c		\
	$(srcdir)/../linker.c		\
	$(srcdir)/../simple.c		\
	$(srcdir)/../compress.c		\
	$(srcdir)/header.sed		\
	$(srcdir)/proto.str		\
	$(srcdir)/../version.h		\
	$(MKDOC)

bfd.h: $(BFD_H_DEP)
	echo "$(BFD_H_DEP)" | sed -f $(srcdir)/header.sed > $@
	for file in $(BFD_H_DEP); do \
	  case $$file in \
	    *-in.h) cat $$file >> $@ ;; \
	    */header.sed) break ;; \
	    *)	echo $$file | sed -e 's,.*/,,' -e 's,^,/* Extracted from ,' \
				-e 's,$$,.  */,' >> $@ ; \
		./$(MKDOC) -f $(srcdir)/proto.str < $$file >> $@ ;; \
	  esac; \
	done
	echo "#ifdef __cplusplus" >> $@
	echo "}" >> $@
	echo "#endif" >> $@
	echo "#endif" >> $@

bfdver.texi: $(srcdir)/Makefile.in
	@echo "creating $@"; \
	echo "@set VERSION $(VERSION)" > bfdver.texi; \
	if test -n "$(PKGVERSION)"; then \
	  echo "@set VERSION_PACKAGE $(PKGVERSION)" >> bfdver.texi; \
	fi; \
	echo "@set UPDATED `date '+%B %Y'`" >> bfdver.texi; \
	if test -n "$(REPORT_BUGS_TEXI)"; then \
	  echo "@set BUGURL $(REPORT_BUGS_TEXI)" >> bfdver.texi; \
	fi

noinst_TEXINFOS = bfdint.texi

MOSTLYCLEANFILES = $(MKDOC) *.o *.stamp

CLEANFILES = *.p *.ip

DISTCLEANFILES = bfd.?? bfd.??? bfd.h libbfd.h libcoff.h texput.log

MAINTAINERCLEANFILES = $(DOCFILES)

# We want install to imply install-info as per GNU standards, despite the
# cygnus option.
install: install-info

MAINTAINERCLEANFILES += bfd.info

# Automake 1.9 will only build info files in the objdir if they are
# mentioned in DISTCLEANFILES.  It doesn't have to be unconditional,
# though, so we use a bogus condition.
if GENINSRC_NEVER
DISTCLEANFILES += bfd.info
endif
