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

AUTOMAKE_OPTIONS = 1.11 no-dist foreign
ACLOCAL_AMFLAGS = -I . -I .. -I ../config

INCDIR = $(srcdir)/../include
CSEARCH = -I. -I$(srcdir) -I$(INCDIR)

SUBDIRS = doc po

bfddocdir = doc

libbfd_la_LDFLAGS =
if INSTALL_LIBBFD
bfdlibdir = @bfdlibdir@
bfdincludedir = @bfdincludedir@
bfdlib_LTLIBRARIES = libbfd.la
bfdinclude_HEADERS = $(BFD_H) $(INCDIR)/ansidecl.h $(INCDIR)/symcat.h $(INCDIR)/bfdlink.h
else !INSTALL_LIBBFD
# Empty these so that the respective installation directories will not be created.
bfdlibdir =
bfdincludedir =
bfdinclude_HEADERS =
rpath_bfdlibdir = @bfdlibdir@
noinst_LTLIBRARIES = libbfd.la
libbfd_la_LDFLAGS += -rpath $(rpath_bfdlibdir)
endif

WARN_CFLAGS = @WARN_CFLAGS@
NO_WERROR = @NO_WERROR@
AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -DBINDIR='"$(bindir)"'
if PLUGINS
bfdinclude_HEADERS += $(INCDIR)/plugin-api.h
LIBDL = @lt_cv_dlopen_libs@
endif

# bfd.h goes here, for now
BFD_H = bfd.h

# Jim Kingdon notes:
# Writing S-records should be included in all (or at least most)
# *-*-coff, *-*-aout, etc., configurations, because people will want to
# be able to use objcopy to create S-records.  (S-records are not useful
# for the debugger, so if you are downloading things as S-records you
# need two copies of the executable, one to download and one for the
# debugger).
BFD32_LIBS = \
	archive.lo archures.lo bfd.lo bfdio.lo bfdwin.lo \
	cache.lo coffgen.lo corefile.lo \
	format.lo init.lo libbfd.lo opncls.lo reloc.lo \
	section.lo syms.lo targets.lo hash.lo linker.lo \
	srec.lo binary.lo tekhex.lo ihex.lo stabs.lo stab-syms.lo \
	merge.lo dwarf2.lo simple.lo compress.lo verilog.lo

BFD64_LIBS = archive64.lo

BFD32_LIBS_CFILES = \
	archive.c archures.c bfd.c bfdio.c bfdwin.c \
	cache.c coffgen.c corefile.c \
	format.c init.c libbfd.c opncls.c reloc.c \
	section.c syms.c targets.c hash.c linker.c \
	srec.c binary.c tekhex.c ihex.c stabs.c stab-syms.c \
	merge.c dwarf2.c simple.c compress.c verilog.c

BFD64_LIBS_CFILES = archive64.c

# This list is alphabetized to make it easier to keep in sync
# with the decls and initializer in archures.c.
ALL_MACHINES = \
	cpu-aarch64.lo \
	cpu-alpha.lo \
	cpu-arc.lo \
	cpu-arm.lo \
	cpu-avr.lo \
	cpu-bfin.lo \
	cpu-cr16.lo \
	cpu-cr16c.lo \
	cpu-cris.lo \
	cpu-crx.lo \
	cpu-d10v.lo \
	cpu-d30v.lo \
	cpu-dlx.lo \
	cpu-epiphany.lo \
	cpu-fr30.lo \
	cpu-frv.lo \
	cpu-h8300.lo \
	cpu-h8500.lo \
	cpu-hppa.lo \
	cpu-i370.lo \
	cpu-i386.lo \
	cpu-l1om.lo \
	cpu-k1om.lo \
	cpu-i860.lo \
	cpu-i960.lo \
	cpu-ia64.lo \
	cpu-ip2k.lo \
	cpu-iq2000.lo \
	cpu-lm32.lo \
	cpu-m10200.lo \
	cpu-m10300.lo \
	cpu-m32c.lo \
	cpu-m32r.lo \
	cpu-m68hc11.lo \
	cpu-m68hc12.lo \
	cpu-m9s12x.lo \
	cpu-m9s12xg.lo \
	cpu-m68k.lo \
	cpu-m88k.lo \
	cpu-mcore.lo \
	cpu-mep.lo \
	cpu-metag.lo \
	cpu-microblaze.lo \
	cpu-mips.lo \
	cpu-mmix.lo \
	cpu-moxie.lo \
	cpu-msp430.lo \
	cpu-mt.lo \
	cpu-nds32.lo \
	cpu-nios2.lo \
	cpu-ns32k.lo \
	cpu-or1k.lo \
	cpu-pdp11.lo \
	cpu-pj.lo \
	cpu-plugin.lo \
	cpu-powerpc.lo \
	cpu-rs6000.lo \
	cpu-rl78.lo \
	cpu-rx.lo \
	cpu-s390.lo \
	cpu-score.lo \
	cpu-sh.lo \
	cpu-sparc.lo \
	cpu-spu.lo \
	cpu-tic30.lo \
	cpu-tic4x.lo \
	cpu-tic54x.lo \
	cpu-tic6x.lo \
	cpu-tic80.lo \
	cpu-tilegx.lo \
	cpu-tilepro.lo \
	cpu-v850.lo \
	cpu-v850_rh850.lo \
	cpu-vax.lo \
	cpu-w65.lo \
	cpu-we32k.lo \
	cpu-xc16x.lo \
	cpu-xgate.lo \
	cpu-xstormy16.lo \
	cpu-xtensa.lo \
	cpu-z80.lo \
	cpu-z8k.lo

ALL_MACHINES_CFILES = \
	cpu-aarch64.c \
	cpu-alpha.c \
	cpu-arc.c \
	cpu-arm.c \
	cpu-avr.c \
	cpu-bfin.c \
	cpu-cr16.c \
	cpu-cr16c.c \
	cpu-cris.c \
	cpu-crx.c \
	cpu-d10v.c \
	cpu-d30v.c \
	cpu-dlx.c \
	cpu-epiphany.c \
	cpu-fr30.c \
	cpu-frv.c \
	cpu-h8300.c \
	cpu-h8500.c \
	cpu-hppa.c \
	cpu-i370.c \
	cpu-i386.c \
	cpu-l1om.c \
	cpu-k1om.c \
	cpu-i860.c \
	cpu-i960.c \
	cpu-ia64.c \
	cpu-ip2k.c \
	cpu-iq2000.c \
	cpu-lm32.c \
	cpu-m10200.c \
	cpu-m10300.c \
	cpu-m32c.c \
	cpu-m32r.c \
	cpu-m68hc11.c \
	cpu-m68hc12.c \
	cpu-m9s12x.c \
	cpu-m9s12xg.c \
	cpu-m68k.c \
	cpu-m88k.c \
	cpu-mcore.c \
	cpu-mep.c \
	cpu-metag.c \
	cpu-microblaze.c \
	cpu-mips.c \
	cpu-mmix.c \
	cpu-moxie.c \
	cpu-msp430.c \
	cpu-mt.c \
	cpu-nds32.c \
	cpu-ns32k.c \
	cpu-nios2.c \
	cpu-or1k.c \
	cpu-pdp11.c \
	cpu-pj.c \
	cpu-plugin.c \
	cpu-powerpc.c \
	cpu-rs6000.c \
	cpu-rl78.c \
	cpu-rx.c \
	cpu-s390.c \
	cpu-score.c \
	cpu-sh.c \
	cpu-sparc.c \
	cpu-spu.c \
	cpu-tic30.c \
	cpu-tic4x.c \
	cpu-tic54x.c \
	cpu-tic6x.c \
	cpu-tic80.c \
	cpu-tilegx.c \
	cpu-tilepro.c \
	cpu-v850.c \
	cpu-v850_rh850.c \
	cpu-vax.c \
	cpu-w65.c \
	cpu-we32k.c \
	cpu-xc16x.c \
	cpu-xgate.c \
	cpu-xstormy16.c \
	cpu-xtensa.c \
	cpu-z80.c \
	cpu-z8k.c

# The .o files needed by all of the 32 bit vectors that are configured into
# target_vector in targets.c if configured with --enable-targets=all.
BFD32_BACKENDS = \
	aout-adobe.lo \
	aout-arm.lo \
	aout-cris.lo \
	aout-ns32k.lo \
	aout-sparcle.lo \
	aout-tic30.lo \
	aout0.lo \
	aout32.lo \
	armnetbsd.lo \
	bout.lo \
	cf-i386lynx.lo \
	cf-sparclynx.lo \
	coff-apollo.lo \
	coff-arm.lo \
	coff-aux.lo \
	coff-go32.lo \
	coff-h8300.lo \
	coff-h8500.lo \
	coff-i386.lo \
	coff-i860.lo \
	coff-i960.lo \
	coff-m68k.lo \
	coff-m88k.lo \
	coff-mips.lo \
	coff-rs6000.lo \
	coff-sh.lo \
	coff-sparc.lo \
	coff-stgo32.lo \
	coff-svm68k.lo \
	coff-tic30.lo \
	coff-tic4x.lo \
	coff-tic54x.lo \
	coff-tic80.lo \
	coff-u68k.lo \
	coff-w65.lo \
	coff-we32k.lo \
	coff-z80.lo \
	coff-z8k.lo \
	cofflink.lo \
	dwarf1.lo \
	ecoff.lo \
	ecofflink.lo \
	elf-attrs.lo \
	elf-eh-frame.lo \
	elf-ifunc.lo \
	elf-m10200.lo \
	elf-m10300.lo \
	elf-nacl.lo \
	elf-strtab.lo \
	elf-vxworks.lo \
	elf.lo \
	elf32-am33lin.lo \
	elf32-arc.lo \
	elf32-arm.lo \
	elf32-avr.lo \
	elf32-bfin.lo \
	elf32-cr16.lo \
	elf32-cr16c.lo \
	elf32-cris.lo \
	elf32-crx.lo \
	elf32-d10v.lo \
	elf32-d30v.lo \
	elf32-dlx.lo \
	elf32-epiphany.lo \
	elf32-fr30.lo \
	elf32-frv.lo \
	elf32-gen.lo \
	elf32-h8300.lo \
	elf32-hppa.lo \
	elf32-i370.lo \
	elf32-i386.lo \
	elf32-i860.lo \
	elf32-i960.lo \
	elf32-ip2k.lo \
	elf32-iq2000.lo \
	elf32-lm32.lo \
	elf32-m32c.lo \
	elf32-m32r.lo \
	elf32-m68hc11.lo \
	elf32-m68hc12.lo \
	elf32-m68hc1x.lo \
	elf32-m68k.lo \
	elf32-m88k.lo \
	elf32-mcore.lo \
	elf32-mep.lo \
	elf32-metag.lo \
	elf32-microblaze.lo \
	elf32-moxie.lo \
	elf32-msp430.lo \
	elf32-mt.lo \
	elf32-nds32.lo \
	elf32-nios2.lo \
	elf32-or1k.lo \
	elf32-pj.lo \
	elf32-ppc.lo \
	elf32-rl78.lo \
	elf32-rx.lo \
	elf32-s390.lo \
	elf32-sh-symbian.lo \
	elf32-sh.lo \
	elf32-sh64-com.lo \
	elf32-sh64.lo \
	elf32-sparc.lo \
	elf32-spu.lo \
	elf32-tic6x.lo \
	elf32-tilegx.lo \
	elf32-tilepro.lo \
	elf32-v850.lo \
	elf32-vax.lo \
	elf32-xc16x.lo \
	elf32-xgate.lo \
	elf32-xstormy16.lo \
	elf32-xtensa.lo \
	elf32.lo \
	elflink.lo \
	elfxx-sparc.lo \
	elfxx-tilegx.lo \
	epoc-pe-arm.lo \
	epoc-pei-arm.lo \
	hp300bsd.lo \
	hp300hpux.lo \
	i386aout.lo \
	i386bsd.lo \
	i386dynix.lo \
	i386freebsd.lo \
	i386linux.lo \
	i386lynx.lo \
	i386mach3.lo \
	i386msdos.lo \
	i386netbsd.lo \
	i386os9k.lo \
	ieee.lo \
	m68k4knetbsd.lo \
	m68klinux.lo \
	m68knetbsd.lo \
	m88kmach3.lo \
	m88kopenbsd.lo \
	mach-o.lo \
	mach-o-i386.lo \
	mipsbsd.lo \
	newsos3.lo \
	nlm.lo \
	nlm32-i386.lo \
	nlm32-ppc.lo \
	nlm32-sparc.lo \
	nlm32.lo \
	ns32knetbsd.lo \
	oasys.lo \
	pc532-mach.lo \
	pdp11.lo \
	pe-arm-wince.lo \
	pe-arm.lo \
	pe-i386.lo \
	pe-mcore.lo \
	pe-mips.lo \
	pe-ppc.lo \
	pe-sh.lo \
	pef.lo \
	pei-arm-wince.lo \
	pei-arm.lo \
	pei-i386.lo \
	pei-mcore.lo \
	pei-mips.lo \
	pei-ppc.lo \
	pei-sh.lo \
	peigen.lo \
	plugin.lo \
	ppcboot.lo \
	reloc16.lo \
	riscix.lo \
	som.lo \
	sparclinux.lo \
	sparclynx.lo \
	sparcnetbsd.lo \
	sunos.lo \
	vax1knetbsd.lo \
	vaxbsd.lo \
	vaxnetbsd.lo \
	versados.lo \
	vms-lib.lo \
	vms-misc.lo \
	xcofflink.lo \
	xsym.lo \
	xtensa-isa.lo \
	xtensa-modules.lo

BFD32_BACKENDS_CFILES = \
	aout-adobe.c \
	aout-arm.c \
	aout-cris.c \
	aout-ns32k.c \
	aout-sparcle.c \
	aout-tic30.c \
	aout0.c \
	aout32.c \
	armnetbsd.c \
	bout.c \
	cf-i386lynx.c \
	cf-sparclynx.c \
	coff-apollo.c \
	coff-arm.c \
	coff-aux.c \
	coff-go32.c \
	coff-h8300.c \
	coff-h8500.c \
	coff-i386.c \
	coff-i860.c \
	coff-i960.c \
	coff-m68k.c \
	coff-m88k.c \
	coff-mips.c \
	coff-rs6000.c \
	coff-sh.c \
	coff-sparc.c \
	coff-stgo32.c \
	coff-svm68k.c \
	coff-tic30.c \
	coff-tic4x.c \
	coff-tic54x.c \
	coff-tic80.c \
	coff-u68k.c \
	coff-w65.c \
	coff-we32k.c \
	coff-z80.c \
	coff-z8k.c \
	cofflink.c \
	dwarf1.c \
	ecoff.c \
	ecofflink.c \
	elf-attrs.c \
	elf-eh-frame.c \
	elf-ifunc.c \
	elf-m10200.c \
	elf-m10300.c \
	elf-nacl.c \
	elf-strtab.c \
	elf-vxworks.c \
	elf.c \
	elf32-am33lin.c \
	elf32-arc.c \
	elf32-arm.c \
	elf32-avr.c \
	elf32-bfin.c \
	elf32-cr16.c \
	elf32-cr16c.c \
	elf32-cris.c \
	elf32-crx.c \
	elf32-d10v.c \
	elf32-d30v.c \
	elf32-dlx.c \
	elf32-epiphany.c \
	elf32-fr30.c \
	elf32-frv.c \
	elf32-gen.c \
	elf32-h8300.c \
	elf32-hppa.c \
	elf32-i370.c \
	elf32-i386.c \
	elf32-i860.c \
	elf32-i960.c \
	elf32-ip2k.c \
	elf32-iq2000.c \
	elf32-lm32.c \
	elf32-m32c.c \
	elf32-m32r.c \
	elf32-m68hc11.c \
	elf32-m68hc12.c \
	elf32-m68hc1x.c \
	elf32-m68k.c \
	elf32-m88k.c \
	elf32-mcore.c \
	elf32-mep.c \
	elf32-metag.c \
	elf32-microblaze.c \
	elf32-moxie.c \
	elf32-msp430.c \
	elf32-mt.c \
	elf32-nds32.c \
	elf32-nios2.c \
	elf32-or1k.c \
	elf32-pj.c \
	elf32-ppc.c \
	elf32-rl78.c \
	elf32-rx.c \
	elf32-s390.c \
	elf32-sh-symbian.c \
	elf32-sh.c \
	elf32-sh64-com.c \
	elf32-sh64.c \
	elf32-sparc.c \
	elf32-spu.c \
	elf32-tic6x.c \
	elf32-tilegx.c \
	elf32-tilepro.c \
	elf32-v850.c \
	elf32-vax.c \
	elf32-xc16x.c \
	elf32-xgate.c \
	elf32-xstormy16.c \
	elf32-xtensa.c \
	elf32.c \
	elflink.c \
	elfxx-sparc.c \
	elfxx-tilegx.c \
	epoc-pe-arm.c \
	epoc-pei-arm.c \
	hp300bsd.c \
	hp300hpux.c \
	i386aout.c \
	i386bsd.c \
	i386dynix.c \
	i386freebsd.c \
	i386linux.c \
	i386lynx.c \
	i386mach3.c \
	i386msdos.c \
	i386netbsd.c \
	i386os9k.c \
	ieee.c \
	m68k4knetbsd.c \
	m68klinux.c \
	m68knetbsd.c \
	m88kmach3.c \
	m88kopenbsd.c \
	mach-o.c \
	mach-o-i386.c \
	mipsbsd.c \
	newsos3.c \
	nlm.c \
	nlm32-i386.c \
	nlm32-ppc.c \
	nlm32-sparc.c \
	nlm32.c \
	ns32knetbsd.c \
	oasys.c \
	pc532-mach.c \
	pdp11.c \
	pe-arm-wince.c \
	pe-arm.c \
	pe-i386.c \
	pe-mcore.c \
	pe-mips.c \
	pe-ppc.c \
	pe-sh.c \
	pef.c \
	pei-arm-wince.c \
	pei-arm.c \
	pei-i386.c \
	pei-mcore.c \
	pei-mips.c \
	pei-ppc.c \
	pei-sh.c \
	plugin.c \
	ppcboot.c \
	reloc16.c \
	riscix.c \
	som.c \
	sparclinux.c \
	sparclynx.c \
	sparcnetbsd.c \
	sunos.c \
	vax1knetbsd.c \
	vaxbsd.c \
	vaxnetbsd.c \
	versados.c \
	vms-lib.c \
	vms-misc.c \
	xcofflink.c \
	xsym.c \
	xtensa-isa.c \
	xtensa-modules.c

# The .o files needed by all of the 64 bit vectors that are configured into
# target_vector in targets.c if configured with --enable-targets=all
# and --enable-64-bit-bfd.
# elf32-ia64.c requires a 64-bit bfd_vma, and hence can not be put in
# BFD32_BACKENDS.
BFD64_BACKENDS = \
	elf32-aarch64.lo \
	elf64-aarch64.lo \
	elfxx-aarch64.lo \
	aix5ppc-core.lo \
	aout64.lo \
	coff-alpha.lo \
	coff-x86_64.lo \
	coff64-rs6000.lo \
	demo64.lo \
	elf32-ia64.lo \
	elf32-mips.lo \
	elf32-score.lo \
	elf32-score7.lo \
	elf64-alpha.lo \
	elf64-gen.lo \
	elf64-hppa.lo \
	elf64-ia64.lo \
	elf64-ia64-vms.lo \
	elf64-mips.lo \
	elf64-mmix.lo \
	elf64-ppc.lo \
	elf64-s390.lo \
	elf64-sh64.lo \
	elf64-sparc.lo \
	elf64-tilegx.lo \
	elf64-x86-64.lo \
	elf64.lo \
	elfn32-mips.lo \
	elfxx-ia64.lo \
	elfxx-mips.lo \
	mach-o-x86-64.lo \
	mmo.lo \
	nlm32-alpha.lo \
	nlm64.lo \
	pe-x86_64.lo \
	pei-ia64.lo \
	pei-x86_64.lo \
	pepigen.lo \
	pex64igen.lo \
	vms-alpha.lo

BFD64_BACKENDS_CFILES = \
	aix5ppc-core.c \
	aout64.c \
	coff-alpha.c \
	coff-x86_64.c \
	coff64-rs6000.c \
	demo64.c \
	elf32-mips.c \
	elf32-score.c \
	elf32-score7.c \
	elf64-alpha.c \
	elf64-gen.c \
	elf64-hppa.c \
	elf64-ia64-vms.c \
	elf64-mips.c \
	elf64-mmix.c \
	elf64-ppc.c \
	elf64-s390.c \
	elf64-sh64.c \
	elf64-sparc.c \
	elf64-tilegx.c \
	elf64-x86-64.c \
	elf64.c \
	elfn32-mips.c \
	elfxx-aarch64.c \
	elfxx-ia64.c \
	elfxx-mips.c \
	mach-o-x86-64.c \
	mmo.c \
	nlm32-alpha.c \
	nlm64.c \
	pe-x86_64.c \
	pei-ia64.c \
	pei-x86_64.c \
	vms-alpha.c

OPTIONAL_BACKENDS = \
	aix386-core.lo \
	cisco-core.lo \
	hpux-core.lo \
	irix-core.lo \
	lynx-core.lo \
	osf-core.lo \
	rs6000-core.lo \
	sco5-core.lo \
	trad-core.lo

OPTIONAL_BACKENDS_CFILES = \
	aix386-core.c \
	cisco-core.c \
	hpux-core.c \
	irix-core.c \
	lynx-core.c \
	osf-core.c \
	rs6000-core.c \
	sco5-core.c \
	trad-core.c

# Reconfigure if config.bfd or configure.host changes.
# development.sh is used to determine -Werror default.
CONFIG_STATUS_DEPENDENCIES = \
	$(srcdir)/config.bfd \
	$(srcdir)/configure.host \
	$(srcdir)/development.sh

# These are defined by configure:
WORDSIZE = @wordsize@
ALL_BACKENDS = @all_backends@
BFD_BACKENDS = @bfd_backends@
BFD_MACHINES = @bfd_machines@
TDEFAULTS = @tdefaults@
HAVEVECS = @havevecs@

INCLUDES = @HDEFINES@ @COREFLAG@ @TDEFINES@ $(CSEARCH) $(CSWITCHES) \
	$(HAVEVECS) @INCINTL@

# C source files that correspond to .o's.
SOURCE_CFILES = \
	$(BFD32_LIBS_CFILES) \
	$(BFD64_LIBS_CFILES) \
	$(ALL_MACHINES_CFILES) \
	$(BFD32_BACKENDS_CFILES) \
	$(BFD64_BACKENDS_CFILES) \
	$(OPTIONAL_BACKENDS_CFILES)

BUILD_CFILES = \
	elf32-aarch64.c elf64-aarch64.c \
	elf32-ia64.c elf64-ia64.c peigen.c pepigen.c pex64igen.c

CFILES = $(SOURCE_CFILES) $(BUILD_CFILES)

## This is a list of all .h files which are in the source tree.
SOURCE_HFILES = \
	aout-target.h aoutf1.h aoutx.h coffcode.h coffswap.h ecoffswap.h \
	elf-bfd.h elf-hppa.h elf-linux-psinfo.h elf32-hppa.h \
	elf64-hppa.h elfcode.h elfcore.h \
	freebsd.h genlink.h go32stub.h \
	libaout.h libbfd.h libcoff.h libecoff.h libhppa.h libieee.h \
	libnlm.h liboasys.h libpei.h libxcoff.h mach-o.h \
	netbsd.h nlm-target.h nlmcode.h nlmswap.h ns32k.h \
	pef.h pef-traceback.h peicode.h som.h version.h \
	vms.h xsym.h

## ... and all .h files which are in the build tree.
BUILD_HFILES = \
	bfdver.h elf32-target.h elf64-target.h targmatch.h bfd_stdint.h

# Ensure they are built early:
BUILT_SOURCES = $(BUILD_HFILES)

HFILES = $(SOURCE_HFILES) $(BUILD_HFILES)

SRC_POTFILES = $(SOURCE_CFILES) $(SOURCE_HFILES)
BLD_POTFILES = $(BUILD_CFILES) $(BUILD_HFILES)

po/SRC-POTFILES.in: @MAINT@ Makefile $(SRC_POTFILES)
	for file in $(SRC_POTFILES); do echo $$file; done \
	  | LC_ALL=C sort > tmp.src \
	  && mv tmp.src $(srcdir)/po/SRC-POTFILES.in

po/BLD-POTFILES.in: @MAINT@ Makefile $(BLD_POTFILES)
	for file in $(BLD_POTFILES); do echo $$file; done \
	  | LC_ALL=C sort > tmp.bld \
	  && mv tmp.bld $(srcdir)/po/BLD-POTFILES.in

all diststuff: info

# Various kinds of .o files to put in libbfd.a:
# BFD_BACKENDS	Routines the configured targets need.
# BFD_MACHINES	Architecture-specific routines the configured targets need.
# COREFILE	Core file routines for a native configuration
# bfd64_libs	Routines for 64bit support
OFILES = $(BFD_BACKENDS) $(BFD_MACHINES) @COREFILE@ @bfd64_libs@

stamp-ofiles: Makefile
	rm -f tofiles
	f=""; \
	for i in $(OFILES) ; do \
	  case " $$f " in \
	    *" $$i "*) ;; \
	    *) f="$$f $$i" ;; \
	  esac ; \
	done ; \
	echo $$f > tofiles
	$(SHELL) $(srcdir)/../move-if-change tofiles ofiles
	touch stamp-ofiles

ofiles: stamp-ofiles ; @true

# Since BFD64_LIBS is optional and we can't have substitution in
# libbfd_la_SOURCES, we put BFD64_LIBS in OFILES instead.
# However, list all sources in EXTRA_libbfd_la_SOURCES so the
# dependency tracking fragments are picked up in the Makefile.
libbfd_la_SOURCES = $(BFD32_LIBS_CFILES)
EXTRA_libbfd_la_SOURCES = $(CFILES)
libbfd_la_DEPENDENCIES = $(OFILES) ofiles
libbfd_la_LIBADD = `cat ofiles` @SHARED_LIBADD@ $(LIBDL)
libbfd_la_LDFLAGS += -release `cat libtool-soversion` @SHARED_LDFLAGS@

# libtool will build .libs/libbfd.a.  We create libbfd.a in the build
# directory so that we don't have to convert all the programs that use
# libbfd.a simultaneously.  This is a hack which should be removed if
# everything else starts using libtool.  FIXME.

noinst_LIBRARIES = libbfd.a
libbfd_a_SOURCES =

stamp-lib: libbfd.la
	libtooldir=`$(LIBTOOL) --config | sed -n -e 's/^objdir=//p'`; \
	if [ -f $$libtooldir/libbfd.a ]; then \
	  cp $$libtooldir/libbfd.a libbfd.tmp; \
	  $(RANLIB) libbfd.tmp; \
	  $(SHELL) $(srcdir)/../move-if-change libbfd.tmp libbfd.a; \
	else true; fi
	touch stamp-lib

libbfd.a: stamp-lib ; @true

# This file holds an array associating configuration triplets and
# vector names.  It is built from config.bfd.  It is not compiled by
# itself, but is included by targets.c.
targmatch.h: config.bfd targmatch.sed
	rm -f targmatch.h
	sed -f $(srcdir)/targmatch.sed < $(srcdir)/config.bfd > targmatch.new
	mv -f targmatch.new targmatch.h

# When compiling archures.c and targets.c, supply the default target
# info from configure.

targets.lo: targets.c Makefile
if am__fastdepCC
	$(LTCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $(TDEFAULTS) $(srcdir)/targets.c
	mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Plo
else
if AMDEP
	source='targets.c' object='$@' libtool=yes @AMDEPBACKSLASH@
	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
endif
	$(LTCOMPILE) -c -o $@ $(TDEFAULTS) $(srcdir)/targets.c
endif

archures.lo: archures.c Makefile
if am__fastdepCC
	$(LTCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $(TDEFAULTS) $(srcdir)/archures.c
	mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Plo
else
if AMDEP
	source='archures.c' object='$@' libtool=yes @AMDEPBACKSLASH@
	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
endif
	$(LTCOMPILE) -c -o $@ $(TDEFAULTS) $(srcdir)/archures.c
endif

dwarf2.lo: dwarf2.c Makefile
if am__fastdepCC
	$(LTCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ -DDEBUGDIR=\"$(DEBUGDIR)\" $(srcdir)/dwarf2.c
	mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Plo
else
if AMDEP
	source='dwarf2.c' object='$@' libtool=yes @AMDEPBACKSLASH@
	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
endif
	$(LTCOMPILE) -c -o $@ -DDEBUGDIR=\"$(DEBUGDIR)\" $(srcdir)/dwarf2.c
endif

elf32-target.h : elfxx-target.h
	rm -f elf32-target.h
	sed -e s/NN/32/g < $(srcdir)/elfxx-target.h > elf32-target.new
	mv -f elf32-target.new elf32-target.h

elf64-target.h : elfxx-target.h
	rm -f elf64-target.h
	sed -e s/NN/64/g < $(srcdir)/elfxx-target.h > elf64-target.new
	mv -f elf64-target.new elf64-target.h

elf32-aarch64.c : elfnn-aarch64.c
	rm -f elf32-aarch64.c
	echo "#line 1 \"$(srcdir)/elfnn-aarch64.c\"" > elf32-aarch64.new
	sed -e s/NN/32/g < $(srcdir)/elfnn-aarch64.c >> elf32-aarch64.new
	mv -f elf32-aarch64.new elf32-aarch64.c

elf64-aarch64.c : elfnn-aarch64.c
	rm -f elf64-aarch64.c
	echo "#line 1 \"$(srcdir)/elfnn-aarch64.c\"" > elf64-aarch64.new
	sed -e s/NN/64/g < $(srcdir)/elfnn-aarch64.c >> elf64-aarch64.new
	mv -f elf64-aarch64.new elf64-aarch64.c

elf32-ia64.c : elfnn-ia64.c
	rm -f elf32-ia64.c
	sed -e s/NN/32/g < $(srcdir)/elfnn-ia64.c > elf32-ia64.new
	mv -f elf32-ia64.new elf32-ia64.c

elf64-ia64.c : elfnn-ia64.c
	rm -f elf64-ia64.c
	sed -e s/NN/64/g < $(srcdir)/elfnn-ia64.c > elf64-ia64.new
	mv -f elf64-ia64.new elf64-ia64.c

peigen.c : peXXigen.c
	rm -f peigen.c
	sed -e s/XX/pe/g < $(srcdir)/peXXigen.c > peigen.new
	mv -f peigen.new peigen.c

pepigen.c : peXXigen.c
	rm -f pepigen.c
	sed -e s/XX/pep/g < $(srcdir)/peXXigen.c > pepigen.new
	mv -f pepigen.new pepigen.c

pex64igen.c: peXXigen.c
	rm -f pex64igen.c
	sed -e s/XX/pex64/g < $(srcdir)/peXXigen.c > pex64igen.new
	mv -f pex64igen.new pex64igen.c

BFD_H_DEPS= $(INCDIR)/ansidecl.h $(INCDIR)/symcat.h
LOCAL_H_DEPS= libbfd.h sysdep.h config.h
$(BFD32_LIBS) \
 $(BFD64_LIBS) \
 $(ALL_MACHINES) \
 $(BFD32_BACKENDS) \
 $(BFD64_BACKENDS) \
 $(OPTIONAL_BACKENDS): $(BFD_H) $(BFD_H_DEPS) $(LOCAL_H_DEPS)

host-aout.lo: Makefile

# The following program can be used to generate a simple config file
# which can be folded into an h-XXX file for a new host, with some editing.
aout-params.h: gen-aout
	./gen-aout host > aout-params.h
gen-aout: $(srcdir)/gen-aout.c Makefile
	$(CC) -o gen-aout $(CFLAGS) $(LFLAGS) $(srcdir)/gen-aout.c

$(BFD_H): stmp-bfd-h ; @true

stmp-bfd-h: bfd-in3.h
	rm -f bfd-tmp.h
	cp bfd-in3.h bfd-tmp.h
	$(SHELL) $(srcdir)/../move-if-change bfd-tmp.h $(BFD_H)
	rm -f bfd-tmp.h
	touch stmp-bfd-h

BFD_H_FILES = bfd-in.h init.c opncls.c libbfd.c section.c archures.c \
	reloc.c syms.c bfd.c bfdio.c bfdwin.c \
	archive.c corefile.c targets.c format.c compress.c
BFD64_H_FILES = archive64.c
LIBBFD_H_FILES = libbfd-in.h init.c libbfd.c bfdio.c bfdwin.c \
	cache.c reloc.c archures.c elf.c
LIBCOFF_H_FILES = libcoff-in.h coffcode.h

# Could really use a "copy-if-change"...
headers:
	(cd $(bfddocdir); $(MAKE) protos $(FLAGS_TO_PASS))
	cp $(bfddocdir)/bfd.h bfd-in2.h-new
	$(SHELL) $(srcdir)/../move-if-change bfd-in2.h-new $(srcdir)/bfd-in2.h
	cp $(bfddocdir)/libbfd.h libbfd.h-new
	$(SHELL) $(srcdir)/../move-if-change libbfd.h-new $(srcdir)/libbfd.h
	cp $(bfddocdir)/libcoff.h libcoff.h-new
	$(SHELL) $(srcdir)/../move-if-change libcoff.h-new $(srcdir)/libcoff.h

# We only rebuild the header files automatically if we have been
# configured with --enable-maintainer-mode.

$(srcdir)/bfd-in2.h: @MAINT@ stmp-bin2-h ; @true
stmp-bin2-h: $(BFD_H_FILES) $(BFD64_H_FILES)
	(cd $(bfddocdir); $(MAKE) $(FLAGS_TO_PASS) bfd.h)
	cp $(bfddocdir)/bfd.h bfd-in2.h-new
	$(SHELL) $(srcdir)/../move-if-change bfd-in2.h-new $(srcdir)/bfd-in2.h
	touch stmp-bin2-h

$(srcdir)/libbfd.h: @MAINT@ stmp-lbfd-h ; @true
stmp-lbfd-h: $(LIBBFD_H_FILES)
	(cd $(bfddocdir); $(MAKE) $(FLAGS_TO_PASS) libbfd.h)
	cp $(bfddocdir)/libbfd.h libbfd.h-new
	$(SHELL) $(srcdir)/../move-if-change libbfd.h-new $(srcdir)/libbfd.h
	touch stmp-lbfd-h

$(srcdir)/libcoff.h: @MAINT@ stmp-lcoff-h ; @true
stmp-lcoff-h: $(LIBCOFF_H_FILES)
	(cd $(bfddocdir); $(MAKE) $(FLAGS_TO_PASS) libcoff.h)
	cp $(bfddocdir)/libcoff.h libcoff.h-new
	$(SHELL) $(srcdir)/../move-if-change libcoff.h-new $(srcdir)/libcoff.h
	touch stmp-lcoff-h

MOSTLYCLEANFILES = ofiles stamp-ofiles

CLEANFILES = bfd.h dep.sed stmp-bfd-h DEP DEPA DEP1 DEP2 libbfd.a stamp-lib \
	stmp-bin2-h stmp-lbfd-h stmp-lcoff-h

DISTCLEANFILES = $(BUILD_CFILES) $(BUILD_HFILES) libtool-soversion

bfdver.h: $(srcdir)/version.h $(srcdir)/development.sh $(srcdir)/Makefile.in
	@echo "creating $@"
	@bfd_version=`echo "$(VERSION)" | sed -e 's/\([^\.]*\)\.*\([^\.]*\)\.*\([^\.]*\)\.*\([^\.]*\)\.*\([^\.]*\).*/\1.00\2.00\3.00\4.00\5/' -e 's/\([^\.]*\)\..*\(..\)\..*\(..\)\..*\(..\)\..*\(..\)$$/\1\2\3\4\5/'` ;\
	bfd_version_string="\"$(VERSION)\"" ;\
	bfd_soversion="$(VERSION)" ;\
	bfd_version_package="\"$(PKGVERSION)\"" ;\
	report_bugs_to="\"$(REPORT_BUGS_TO)\"" ;\
	. $(srcdir)/development.sh ;\
	if test "$$development" = true ; then \
	  bfd_version_date=`sed -n -e 's/.*DATE //p' < $(srcdir)/version.h` ;\
	  bfd_version_string="\"$(VERSION).$${bfd_version_date}\"" ;\
	  bfd_soversion="$(VERSION).$${bfd_version_date}" ;\
	fi ;\
	sed -e "s,@bfd_version@,$$bfd_version," \
	    -e "s,@bfd_version_string@,$$bfd_version_string," \
	    -e "s,@bfd_version_package@,$$bfd_version_package," \
	    -e "s,@report_bugs_to@,$$report_bugs_to," \
	    < $(srcdir)/version.h > $@; \
	echo "$${bfd_soversion}" > libtool-soversion

# Disable -Werror, if it has been enabled, since coffswap.h won't
# compile with gcc 4.5 and above.
coff-tic4x.lo: coff-tic4x.c
@am__fastdepCC_TRUE@	$(LTCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $< $(NO_WERROR)
@am__fastdepCC_TRUE@	$(am__mv) $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Plo
@AMDEP_TRUE@@am__fastdepCC_FALSE@	source='$<' object='$@' libtool=yes @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCC_FALSE@	$(LTCOMPILE) -c -o $@ $< $(NO_WERROR)

coff-tic54x.lo: coff-tic54x.c
@am__fastdepCC_TRUE@	$(LTCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $< $(NO_WERROR)
@am__fastdepCC_TRUE@	$(am__mv) $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Plo
@AMDEP_TRUE@@am__fastdepCC_FALSE@	source='$<' object='$@' libtool=yes @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCC_FALSE@	$(LTCOMPILE) -c -o $@ $< $(NO_WERROR)
