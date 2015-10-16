# Makefile for program source directory in GNU NLS utilities package.
# Copyright (C) 1995, 1996, 1997 by Ulrich Drepper <drepper@gnu.ai.mit.edu>
# Copyright (C) 2001-2014 Free Software Foundation, Inc.
#
# This file may be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

PACKAGE = bfd
VERSION = 2.25.1

SHELL = /bin/sh


srcdir = .
top_srcdir = ..

top_builddir = ..

prefix = /usr/local
exec_prefix = ${prefix}
datadir = $(prefix)/
localedir = $(datadir)/locale
gnulocaledir = $(prefix)/share/locale
gettextsrcdir = $(prefix)/share/gettext/po
subdir = po

DESTDIR =

INSTALL = /opt/local/bin/ginstall -c
INSTALL_DATA = ${INSTALL} -m 644
MKINSTALLDIRS = $(top_builddir)/./../mkinstalldirs

CC = gcc
GENCAT = 
GMSGFMT = PATH=../src:$$PATH /opt/local/bin/msgfmt
MSGFMT = /opt/local/bin/msgfmt
XGETTEXT = PATH=../src:$$PATH /opt/local/bin/xgettext
MSGMERGE = PATH=../src:$$PATH msgmerge

DEFS = -DHAVE_CONFIG_H
CFLAGS = -g -O2
CPPFLAGS = 

INCLUDES = -I.. -I$(top_srcdir)/intl

COMPILE = $(CC) -c $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(XCFLAGS)

SOURCES = cat-id-tbl.c
POFILES =  fr.po tr.po ja.po es.po sv.po da.po zh_CN.po ro.po rw.po vi.po fi.po id.po ru.po uk.po
GMOFILES =  fr.gmo tr.gmo ja.gmo es.gmo sv.gmo da.gmo zh_CN.gmo ro.gmo rw.gmo vi.gmo fi.gmo id.gmo ru.gmo uk.gmo
DISTFILES = ChangeLog Makefile.in.in SRC-POTFILES.in BLD-POTFILES.in $(PACKAGE).pot \
stamp-cat-id $(POFILES) $(GMOFILES) $(SOURCES)

# Note - the following line gets processed by bfd/configure and amended
# to contain the full list of source dir POTFILES.
SRC-POTFILES = \
	../aix386-core.c \
	../aix5ppc-core.c \
	../aout-adobe.c \
	../aout-arm.c \
	../aout-cris.c \
	../aout-ns32k.c \
	../aout-sparcle.c \
	../aout-target.h \
	../aout-tic30.c \
	../aout0.c \
	../aout32.c \
	../aout64.c \
	../aoutf1.h \
	../aoutx.h \
	../archive.c \
	../archive64.c \
	../archures.c \
	../armnetbsd.c \
	../bfd.c \
	../bfdio.c \
	../bfdwin.c \
	../binary.c \
	../bout.c \
	../cache.c \
	../cf-i386lynx.c \
	../cf-sparclynx.c \
	../cisco-core.c \
	../coff-alpha.c \
	../coff-apollo.c \
	../coff-arm.c \
	../coff-aux.c \
	../coff-go32.c \
	../coff-h8300.c \
	../coff-h8500.c \
	../coff-i386.c \
	../coff-i860.c \
	../coff-i960.c \
	../coff-m68k.c \
	../coff-m88k.c \
	../coff-mips.c \
	../coff-rs6000.c \
	../coff-sh.c \
	../coff-sparc.c \
	../coff-stgo32.c \
	../coff-svm68k.c \
	../coff-tic30.c \
	../coff-tic4x.c \
	../coff-tic54x.c \
	../coff-tic80.c \
	../coff-u68k.c \
	../coff-w65.c \
	../coff-we32k.c \
	../coff-x86_64.c \
	../coff-z80.c \
	../coff-z8k.c \
	../coff64-rs6000.c \
	../coffcode.h \
	../coffgen.c \
	../cofflink.c \
	../coffswap.h \
	../compress.c \
	../corefile.c \
	../cpu-aarch64.c \
	../cpu-alpha.c \
	../cpu-arc.c \
	../cpu-arm.c \
	../cpu-avr.c \
	../cpu-bfin.c \
	../cpu-cr16.c \
	../cpu-cr16c.c \
	../cpu-cris.c \
	../cpu-crx.c \
	../cpu-d10v.c \
	../cpu-d30v.c \
	../cpu-dlx.c \
	../cpu-epiphany.c \
	../cpu-fr30.c \
	../cpu-frv.c \
	../cpu-h8300.c \
	../cpu-h8500.c \
	../cpu-hppa.c \
	../cpu-i370.c \
	../cpu-i386.c \
	../cpu-i860.c \
	../cpu-i960.c \
	../cpu-ia64.c \
	../cpu-ip2k.c \
	../cpu-iq2000.c \
	../cpu-k1om.c \
	../cpu-l1om.c \
	../cpu-lm32.c \
	../cpu-m10200.c \
	../cpu-m10300.c \
	../cpu-m32c.c \
	../cpu-m32r.c \
	../cpu-m68hc11.c \
	../cpu-m68hc12.c \
	../cpu-m68k.c \
	../cpu-m88k.c \
	../cpu-m9s12x.c \
	../cpu-m9s12xg.c \
	../cpu-mcore.c \
	../cpu-mep.c \
	../cpu-metag.c \
	../cpu-microblaze.c \
	../cpu-mips.c \
	../cpu-mmix.c \
	../cpu-moxie.c \
	../cpu-msp430.c \
	../cpu-mt.c \
	../cpu-nds32.c \
	../cpu-nios2.c \
	../cpu-ns32k.c \
	../cpu-or1k.c \
	../cpu-pdp11.c \
	../cpu-pj.c \
	../cpu-plugin.c \
	../cpu-powerpc.c \
	../cpu-rl78.c \
	../cpu-rs6000.c \
	../cpu-rx.c \
	../cpu-s390.c \
	../cpu-score.c \
	../cpu-sh.c \
	../cpu-sparc.c \
	../cpu-spu.c \
	../cpu-tic30.c \
	../cpu-tic4x.c \
	../cpu-tic54x.c \
	../cpu-tic6x.c \
	../cpu-tic80.c \
	../cpu-tilegx.c \
	../cpu-tilepro.c \
	../cpu-v850.c \
	../cpu-v850_rh850.c \
	../cpu-vax.c \
	../cpu-w65.c \
	../cpu-we32k.c \
	../cpu-xc16x.c \
	../cpu-xgate.c \
	../cpu-xstormy16.c \
	../cpu-xtensa.c \
	../cpu-z80.c \
	../cpu-z8k.c \
	../demo64.c \
	../dwarf1.c \
	../dwarf2.c \
	../ecoff.c \
	../ecofflink.c \
	../ecoffswap.h \
	../elf-attrs.c \
	../elf-bfd.h \
	../elf-eh-frame.c \
	../elf-hppa.h \
	../elf-ifunc.c \
	../elf-linux-psinfo.h \
	../elf-m10200.c \
	../elf-m10300.c \
	../elf-nacl.c \
	../elf-strtab.c \
	../elf-vxworks.c \
	../elf.c \
	../elf32-am33lin.c \
	../elf32-arc.c \
	../elf32-arm.c \
	../elf32-avr.c \
	../elf32-bfin.c \
	../elf32-cr16.c \
	../elf32-cr16c.c \
	../elf32-cris.c \
	../elf32-crx.c \
	../elf32-d10v.c \
	../elf32-d30v.c \
	../elf32-dlx.c \
	../elf32-epiphany.c \
	../elf32-fr30.c \
	../elf32-frv.c \
	../elf32-gen.c \
	../elf32-h8300.c \
	../elf32-hppa.c \
	../elf32-hppa.h \
	../elf32-i370.c \
	../elf32-i386.c \
	../elf32-i860.c \
	../elf32-i960.c \
	../elf32-ip2k.c \
	../elf32-iq2000.c \
	../elf32-lm32.c \
	../elf32-m32c.c \
	../elf32-m32r.c \
	../elf32-m68hc11.c \
	../elf32-m68hc12.c \
	../elf32-m68hc1x.c \
	../elf32-m68k.c \
	../elf32-m88k.c \
	../elf32-mcore.c \
	../elf32-mep.c \
	../elf32-metag.c \
	../elf32-microblaze.c \
	../elf32-mips.c \
	../elf32-moxie.c \
	../elf32-msp430.c \
	../elf32-mt.c \
	../elf32-nds32.c \
	../elf32-nios2.c \
	../elf32-or1k.c \
	../elf32-pj.c \
	../elf32-ppc.c \
	../elf32-rl78.c \
	../elf32-rx.c \
	../elf32-s390.c \
	../elf32-score.c \
	../elf32-score7.c \
	../elf32-sh-symbian.c \
	../elf32-sh.c \
	../elf32-sh64-com.c \
	../elf32-sh64.c \
	../elf32-sparc.c \
	../elf32-spu.c \
	../elf32-tic6x.c \
	../elf32-tilegx.c \
	../elf32-tilepro.c \
	../elf32-v850.c \
	../elf32-vax.c \
	../elf32-xc16x.c \
	../elf32-xgate.c \
	../elf32-xstormy16.c \
	../elf32-xtensa.c \
	../elf32.c \
	../elf64-alpha.c \
	../elf64-gen.c \
	../elf64-hppa.c \
	../elf64-hppa.h \
	../elf64-ia64-vms.c \
	../elf64-mips.c \
	../elf64-mmix.c \
	../elf64-ppc.c \
	../elf64-s390.c \
	../elf64-sh64.c \
	../elf64-sparc.c \
	../elf64-tilegx.c \
	../elf64-x86-64.c \
	../elf64.c \
	../elfcode.h \
	../elfcore.h \
	../elflink.c \
	../elfn32-mips.c \
	../elfxx-aarch64.c \
	../elfxx-ia64.c \
	../elfxx-mips.c \
	../elfxx-sparc.c \
	../elfxx-tilegx.c \
	../epoc-pe-arm.c \
	../epoc-pei-arm.c \
	../format.c \
	../freebsd.h \
	../genlink.h \
	../go32stub.h \
	../hash.c \
	../hp300bsd.c \
	../hp300hpux.c \
	../hpux-core.c \
	../i386aout.c \
	../i386bsd.c \
	../i386dynix.c \
	../i386freebsd.c \
	../i386linux.c \
	../i386lynx.c \
	../i386mach3.c \
	../i386msdos.c \
	../i386netbsd.c \
	../i386os9k.c \
	../ieee.c \
	../ihex.c \
	../init.c \
	../irix-core.c \
	../libaout.h \
	../libbfd.c \
	../libbfd.h \
	../libcoff.h \
	../libecoff.h \
	../libhppa.h \
	../libieee.h \
	../libnlm.h \
	../liboasys.h \
	../libpei.h \
	../libxcoff.h \
	../linker.c \
	../lynx-core.c \
	../m68k4knetbsd.c \
	../m68klinux.c \
	../m68knetbsd.c \
	../m88kmach3.c \
	../m88kopenbsd.c \
	../mach-o-i386.c \
	../mach-o-x86-64.c \
	../mach-o.c \
	../mach-o.h \
	../merge.c \
	../mipsbsd.c \
	../mmo.c \
	../netbsd.h \
	../newsos3.c \
	../nlm-target.h \
	../nlm.c \
	../nlm32-alpha.c \
	../nlm32-i386.c \
	../nlm32-ppc.c \
	../nlm32-sparc.c \
	../nlm32.c \
	../nlm64.c \
	../nlmcode.h \
	../nlmswap.h \
	../ns32k.h \
	../ns32knetbsd.c \
	../oasys.c \
	../opncls.c \
	../osf-core.c \
	../pc532-mach.c \
	../pdp11.c \
	../pe-arm-wince.c \
	../pe-arm.c \
	../pe-i386.c \
	../pe-mcore.c \
	../pe-mips.c \
	../pe-ppc.c \
	../pe-sh.c \
	../pe-x86_64.c \
	../pef-traceback.h \
	../pef.c \
	../pef.h \
	../pei-arm-wince.c \
	../pei-arm.c \
	../pei-i386.c \
	../pei-ia64.c \
	../pei-mcore.c \
	../pei-mips.c \
	../pei-ppc.c \
	../pei-sh.c \
	../pei-x86_64.c \
	../peicode.h \
	../plugin.c \
	../ppcboot.c \
	../reloc.c \
	../reloc16.c \
	../riscix.c \
	../rs6000-core.c \
	../sco5-core.c \
	../section.c \
	../simple.c \
	../som.c \
	../som.h \
	../sparclinux.c \
	../sparclynx.c \
	../sparcnetbsd.c \
	../srec.c \
	../stab-syms.c \
	../stabs.c \
	../sunos.c \
	../syms.c \
	../targets.c \
	../tekhex.c \
	../trad-core.c \
	../vax1knetbsd.c \
	../vaxbsd.c \
	../vaxnetbsd.c \
	../verilog.c \
	../versados.c \
	../version.h \
	../vms-alpha.c \
	../vms-lib.c \
	../vms-misc.c \
	../vms.h \
	../xcofflink.c \
	../xsym.c \
	../xsym.h \
	../xtensa-isa.c \
	../xtensa-modules.c 

# Note - the following line gets processed by bfd/configure and amended
# to contain the full list of build dir POTFILES.
BLD-POTFILES = \
	../bfd_stdint.h \
	../bfdver.h \
	../elf32-aarch64.c \
	../elf32-ia64.c \
	../elf32-target.h \
	../elf64-aarch64.c \
	../elf64-ia64.c \
	../elf64-target.h \
	../peigen.c \
	../pepigen.c \
	../pex64igen.c \
	../targmatch.h 

CATALOGS = 
CATOBJEXT = 
INSTOBJEXT = 

.SUFFIXES:
.SUFFIXES: .c .o .po .pox .gmo .mo .msg .cat

.c.o:
	$(COMPILE) $<

.po.pox:
	$(MAKE) $(PACKAGE).pot
	$(MSGMERGE) $< $(srcdir)/$(PACKAGE).pot -o $*.pox

.po.mo:
	$(MSGFMT) -o $@ $<

.po.gmo:
	file=`echo $* | sed 's,.*/,,'`.gmo \
	  && rm -f $$file && $(GMSGFMT) -o $$file $<

.po.cat:
	sed -f ../intl/po2msg.sed < $< > $*.msg \
	  && rm -f $@ && $(GENCAT) $@ $*.msg


all: all-no

all-yes: $(CATALOGS)  $(PACKAGE).pot
all-no:

$(srcdir)/$(PACKAGE).pot: $(SRC-POTFILES) $(BLD-POTFILES)
	$(XGETTEXT) --default-domain=$(PACKAGE) \
	  --directory=$(top_srcdir) \
	  --add-comments --keyword=_ --keyword=N_ \
	  --msgid-bugs-address=bug-binutils@gnu.org \
	  --files-from=$(srcdir)/SRC-POTFILES.in 
	$(XGETTEXT) --default-domain=$(PACKAGE) \
	  --directory=.. \
	  --directory=. \
	  --add-comments --keyword=_ --keyword=N_ \
	  --join-existing \
	  --msgid-bugs-address=bug-binutils@gnu.org \
	  --files-from=$(srcdir)/BLD-POTFILES.in
	rm -f $(srcdir)/$(PACKAGE).pot
	mv $(PACKAGE).po $(srcdir)/$(PACKAGE).pot

$(srcdir)/cat-id-tbl.c: stamp-cat-id; @:
$(srcdir)/stamp-cat-id: $(PACKAGE).pot
	rm -f cat-id-tbl.tmp
	sed -f ../intl/po2tbl.sed $(srcdir)/$(PACKAGE).pot \
		| sed -e "s/@PACKAGE NAME@/$(PACKAGE)/" > cat-id-tbl.tmp
	if cmp -s cat-id-tbl.tmp $(srcdir)/cat-id-tbl.c; then \
	  rm cat-id-tbl.tmp; \
	else \
	  echo cat-id-tbl.c changed; \
	  rm -f $(srcdir)/cat-id-tbl.c; \
	  mv cat-id-tbl.tmp $(srcdir)/cat-id-tbl.c; \
	fi
	cd $(srcdir) && rm -f stamp-cat-id && echo timestamp > stamp-cat-id


install: install-exec install-data
install-exec:
install-info:
install-html:
install-pdf:
install-data: install-data-no
install-data-no: all
install-data-yes: all
	if test -r $(MKINSTALLDIRS); then \
	  $(MKINSTALLDIRS) $(DESTDIR)$(datadir); \
	else \
	  $(top_srcdir)/mkinstalldirs $(DESTDIR)$(datadir); \
	fi
	@catalogs='$(CATALOGS)'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  case "$$cat" in \
	    *.gmo) destdir=$(gnulocaledir);; \
	    *)     destdir=$(localedir);; \
	  esac; \
	  lang=`echo $$cat | sed 's/\$(CATOBJEXT)$$//'`; \
	  dir=$(DESTDIR)$$destdir/$$lang/LC_MESSAGES; \
	  if test -r $(MKINSTALLDIRS); then \
	    $(MKINSTALLDIRS) $$dir; \
	  else \
	    $(top_srcdir)/mkinstalldirs $$dir; \
	  fi; \
	  if test -r $$cat; then \
	    $(INSTALL_DATA) $$cat $$dir/$(PACKAGE)$(INSTOBJEXT); \
	    echo "installing $$cat as $$dir/$(PACKAGE)$(INSTOBJEXT)"; \
	  else \
	    $(INSTALL_DATA) $(srcdir)/$$cat $$dir/$(PACKAGE)$(INSTOBJEXT); \
	    echo "installing $(srcdir)/$$cat as" \
		 "$$dir/$(PACKAGE)$(INSTOBJEXT)"; \
	  fi; \
	  if test -r $$cat.m; then \
	    $(INSTALL_DATA) $$cat.m $$dir/$(PACKAGE)$(INSTOBJEXT).m; \
	    echo "installing $$cat.m as $$dir/$(PACKAGE)$(INSTOBJEXT).m"; \
	  else \
	    if test -r $(srcdir)/$$cat.m ; then \
	      $(INSTALL_DATA) $(srcdir)/$$cat.m \
		$$dir/$(PACKAGE)$(INSTOBJEXT).m; \
	      echo "installing $(srcdir)/$$cat as" \
		   "$$dir/$(PACKAGE)$(INSTOBJEXT).m"; \
	    else \
	      true; \
	    fi; \
	  fi; \
	done
	if test "$(PACKAGE)" = "gettext"; then \
	  if test -r $(MKINSTALLDIRS); then \
	    $(MKINSTALLDIRS) $(DESTDIR)$(gettextsrcdir); \
	  else \
	    $(top_srcdir)/mkinstalldirs $(DESTDIR)$(gettextsrcdir); \
	  fi; \
	  $(INSTALL_DATA) $(srcdir)/Makefile.in.in \
			  $(DESTDIR)$(gettextsrcdir)/Makefile.in.in; \
	else \
	  : ; \
	fi

# Define this as empty until I found a useful application.
installcheck:

uninstall:
	catalogs='$(CATALOGS)'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed 's/\$(CATOBJEXT)$$//'`; \
	  rm -f $(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(PACKAGE)$(INSTOBJEXT); \
	  rm -f $(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(PACKAGE)$(INSTOBJEXT).m; \
	  rm -f $(DESTDIR)$(gnulocaledir)/$$lang/LC_MESSAGES/$(PACKAGE)$(INSTOBJEXT); \
	  rm -f $(DESTDIR)$(gnulocaledir)/$$lang/LC_MESSAGES/$(PACKAGE)$(INSTOBJEXT).m; \
	done
	rm -f $(DESTDIR)$(gettextsrcdir)/po-Makefile.in.in

check: all

cat-id-tbl.o: ../intl/libgettext.h

html dvi pdf ps info tags TAGS ID:

mostlyclean:
	rm -f core core.* *.pox $(PACKAGE).po *.old.po cat-id-tbl.tmp
	rm -fr *.o

clean: mostlyclean

distclean: clean
	rm -f Makefile Makefile.in *.mo *.msg *.cat *.cat.m
	rm -f SRC-POTFILES BLD-POTFILES 

maintainer-clean: distclean
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."
	rm -f $(GMOFILES) SRC-POTFILES.in BLD-POTFILES.in

distdir = ../$(PACKAGE)-$(VERSION)/$(subdir)
dist distdir: update-po $(DISTFILES)
	dists="$(DISTFILES)"; \
	for file in $$dists; do \
	  ln $(srcdir)/$$file $(distdir) 2> /dev/null \
	    || cp -p $(srcdir)/$$file $(distdir); \
	done

update-po: Makefile
	$(MAKE) $(PACKAGE).pot
	PATH=`pwd`/../src:$$PATH; \
	cd $(srcdir); \
	catalogs='$(CATALOGS)'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed 's/\$(CATOBJEXT)$$//'`; \
	  mv $$lang.po $$lang.old.po; \
	  echo "$$lang:"; \
	  if $(MSGMERGE) $$lang.old.po $(PACKAGE).pot -o $$lang.po; then \
	    rm -f $$lang.old.po; \
	  else \
	    echo "msgmerge for $$cat failed!"; \
	    rm -f $$lang.po; \
	    mv $$lang.old.po $$lang.po; \
	  fi; \
	done

SRC-POTFILES: SRC-POTFILES.in
	( if test 'x$(srcdir)' != 'x.'; then \
	    posrcprefix='$(top_srcdir)/'; \
	  else \
	    posrcprefix="../"; \
	  fi; \
	  rm -f $@-t $@ \
	    && (sed -e '/^#/d' \
	            -e '/^[ 	]*$$/d' \
		    -e "s@.*@	$$posrcprefix& \\\\@" < $(srcdir)/$@.in \
		| sed -e '$$s/\\$$//') > $@-t \
	    && chmod a-w $@-t \
	    && mv $@-t $@ )

BLD-POTFILES: BLD-POTFILES.in
	  ( rm -f $@-t $@ \
	    && (sed -e '/^#/d' \
	            -e '/^[ 	]*$$/d' \
		    -e "s@.*@	../& \\\\@" < $(srcdir)/$@.in \
		| sed -e '$$s/\\$$//') > $@-t \
	    && chmod a-w $@-t \
	    && mv $@-t $@ )

SRC-POTFILES.in:  ../Makefile
	cd .. && $(MAKE) po/SRC-POTFILES.in

BLD-POTFILES.in:  ../Makefile
	cd .. && $(MAKE) po/BLD-POTFILES.in

# Note - The presence of SRC-POTFILES and BLD-POTFILES as dependencies
# here breaks the implementation of the 'distclean' rule for maintainers.
# This is because if 'make distclean' is run in the BFD directory, the
# Makefile there will be deleted before 'distclean' is made here, and so
# the dependency SRC-POTFILES -> SRC-POTFILES.in -> ../Makefile cannot
# be satisfied.
#
# The SRC-POTFILES and BLD-POTFILES dependencies cannot be removed,
# however since it is necessary that these files be built during
# *configure* time, so that configure can insert them into the
# po/Makefile that it is creating, so that the Makefile will have
# the correct dependencies.
Makefile: Make-in ../config.status SRC-POTFILES BLD-POTFILES
	cd .. \
	  && CONFIG_FILES=$(subdir)/Makefile.in:$(subdir)/Make-in \
	     CONFIG_HEADERS= $(SHELL) ./config.status

# Tell versions [3.59,3.63) of GNU make not to export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
