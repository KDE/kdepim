# -*- mode: makefile; coding: utf-8 -*-
# Copyright © 2003 Christopher L Cheney <ccheney@debian.org>
# Description: A class for KDE packages; sets KDE environment variables, etc
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.


ifndef _cdbs_bootstrap
_cdbs_scripts_path ?= /usr/lib/cdbs
_cdbs_rules_path ?= /usr/share/cdbs/1/rules
_cdbs_class_path ?= /usr/share/cdbs/1/class
endif

ifndef _cdbs_class_kde
_cdbs_class_kde := 1

include $(_cdbs_rules_path)/buildcore.mk$(_cdbs_makefile_suffix)

DEB_BUILDDIR = obj-$(DEB_BUILD_GNU_TYPE)

common-configure-arch common-configure-indep:: debian/stamp-cvs-make
debian/stamp-cvs-make:
	if test -n "$(DEB_KDE_CVS_MAKE)" -a ! -f configure; then \
		$(MAKE) -f admin/Makefile.common; \
	fi
	touch debian/stamp-cvs-make

include $(_cdbs_class_path)/autotools.mk$(_cdbs_makefile_suffix)

export kde_cgidir = /usr/lib/cgi-bin
export kde_confdir = /etc/kde3
export kde_htmldir = /usr/share/doc/kde/HTML

ifeq (,$(findstring nofinal,$(DEB_BUILD_OPTIONS)))
	cdbs_kde_enable_final = $(if $(DEB_KDE_ENABLE_FINAL),--enable-final,)
endif
ifneq (,$(findstring nostrip,$(DEB_BUILD_OPTIONS)))
	cdbs_kde_enable_final = 
	cdbs_kde_enable_debug = --enable-debug=full
else
	cdbs_kde_enable_debug = --disable-debug
endif

cdbs_configure_flags += --with-qt-dir=/usr/share/qt3 --disable-rpath --with-xinerama $(cdbs_kde_enable_final) $(cdbs_kde_enable_debug)

DEB_AC_AUX_DIR = $(DEB_SRCDIR)/admin
DEB_CONFIGURE_INCLUDEDIR = "\$${prefix}/include/kde"
DEB_COMPRESS_EXCLUDE := .dcl .docbook -license .tag

common-build-arch common-build-indep:: debian/stamp-apidox
debian/stamp-apidox:
	if test -n "$(DEB_KDE_APIDOX)"; then \
		$(DEB_MAKE_INVOKE) apidox; \
	fi
	touch debian/stamp-apidox

common-install-arch common-install-indep:: install-apidox
install-apidox::
	if test -n "$(DEB_KDE_APIDOX)"; then \
		$(DEB_MAKE_INVOKE) install-apidox DESTDIR=$(DEB_DESTDIR); \
	fi

clean::
	if test -n "$(DEB_KDE_CVS_MAKE)"; then \
		find . -name Makefile.in -print | \
			xargs --no-run-if-empty rm -f; \
		rm -f Makefile.am acinclude.m4 aclocal.m4 config.h.in \
			configure configure.files configure.in stamp-h.in \
			subdirs; \
	fi
	rm -f debian/stamp-cvs-make
	rm -f debian/stamp-apidox

cleanbuilddir::
	-if test "$(DEB_BUILDDIR)" != "$(DEB_SRCDIR)"; then rm -rf $(DEB_BUILDDIR); fi

binary-install/$(DEB_SOURCE_PACKAGE)-doc-html::
	set -e; \
	for doc in `cd debian/tmp/usr/share/doc/kde/HTML/en; find . -name index.docbook`; do \
	  pkg=$${doc%/index.docbook}; pkg=$${pkg#./}; \
	  echo Building $$pkg HTML docs...; \
	  mkdir -p $(CURDIR)/debian/$(DEB_SOURCE_PACKAGE)-doc-html/usr/share/doc/kde/HTML/en/$$pkg; \
	  cd $(CURDIR)/debian/$(DEB_SOURCE_PACKAGE)-doc-html/usr/share/doc/kde/HTML/en/$$pkg; \
	  meinproc $(CURDIR)/debian/tmp/usr/share/doc/kde/HTML/en/$$pkg/index.docbook; \
	done

$(patsubst %,binary-install/%,$(DEB_PACKAGES)) :: binary-install/%:
	if test -x /usr/bin/dh_desktop; then dh_desktop -p$(cdbs_curpkg) $(DEB_DH_DESKTOP_ARGS); fi

endif
