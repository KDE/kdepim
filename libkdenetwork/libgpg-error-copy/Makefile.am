# Makefile.am for libgpg-error.
# Copyright (C) 2003 g10 Code GmbH
# 
# This file is part of libgpg-error.
# 
# libgpg-error is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
# 
# libgpg-error is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

# We distribute the generated sources err-sources.h and err-codes.h,
# because they are needed to build the po directory, and they don't
# depend on the configuration anyway. 
#EXTRA_DIST = mkstrtable.awk err-sources.h.in err-codes.h.in \
#	mkerrnos.awk errnos.in mkerrcodes1.awk mkerrcodes2.awk \
#	err-sources.h err-codes.h gpg-error.m4
#BUILT_SOURCES = err-sources.h err-codes.h code-to-errno.h code-from-errno.h
CLEANFILES = err-sources.h err-codes.h code-to-errno.h code-from-errno.h gpg-error.h mkerrcodes.h mkerrcodes

#bin_SCRIPTS = gpg-error-config
#m4datadir = $(datadir)/aclocal
#m4data_DATA = gpg-error.m4
#
#include_HEADERS = gpg-error.h

noinst_LTLIBRARIES = libgpg-error.la

#libgpg_error_la_LDFLAGS = -version-info @LIBGPG_ERROR_LT_CURRENT@:@LIBGPG_ERROR_LT_REVISION@:@LIBGPG_ERROR_LT_AGE@

libgpg_error_la_SOURCES = strsource.c strerror.c code-to-errno.c code-from-errno.c

#libgpg_error_la_LIBADD = @LTLIBINTL@

#We don't need the gpg-error command-line tool

# Dependencies on generated files.
strsource.lo: err-sources.h gpg-error.h
strerror.lo: err-codes.h gpg-error.h
code-to-errno.lo: code-to-errno.h gpg-error.h
code-from-errno.lo: code-from-errno.h gpg-error.h

err-sources.h: $(srcdir)/mkstrtable.awk $(srcdir)/err-sources.h.in
	$(AWK) -f $(srcdir)/mkstrtable.awk -v textidx=3 \
                $(srcdir)/err-sources.h.in >$@

#We don't need err-sources-sym.h, err-codes-sym.h, errnos-sym.h.
# since those are for the gpg-error binary

err-codes.h: $(srcdir)/mkstrtable.awk err-codes.h.in
	$(AWK) -f $(srcdir)/mkstrtable.awk -v textidx=3 \
                $(srcdir)/err-codes.h.in >$@

code-to-errno.h: mkerrnos.awk errnos.in
	$(AWK) -f $(srcdir)/mkerrnos.awk $(srcdir)/errnos.in >$@

# It is correct to use $(CPP).  We want the host's idea of the error codes.
mkerrcodes.h: mkerrcodes.awk
	$(AWK) -f $(srcdir)/mkerrcodes1.awk $(srcdir)/errnos.in \
        | $(CPP) - | grep GPG_ERR_ | $(AWK) -f $(srcdir)/mkerrcodes.awk >$@

# It is correct to use $(CC_FOR_BUILD) here.  We want to run the
# program at build time.
# DF: we have no such thing in KDE. Using $(CC) instead.
mkerrcodes: mkerrcodes.c mkerrcodes.h
	$(CC) -I. -I$(srcdir) -o $@ $(srcdir)/mkerrcodes.c

code-from-errno.h: mkerrcodes
	./$< | $(AWK) -f $(srcdir)/mkerrcodes2.awk >$@

gpg-error.h: mkheader.awk \
                 err-sources.h.in err-codes.h.in errnos.in gpg-error.h.in
	$(AWK) -f $(srcdir)/mkheader.awk \
                $(srcdir)/err-sources.h.in \
                $(srcdir)/err-codes.h.in \
                $(srcdir)/errnos.in \
                $(srcdir)/gpg-error.h.in > $@

# Removed all dependencies on Makefiles, since for us Makefile is in builddir.
