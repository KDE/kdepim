# Copyright (C) 2000 Werner Koch (dd9jn)
# Copyright (C) 2001, 2002, 2003 g10 Code GmbH
# 
# This file is part of GPGME.
# 
# GPGME is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# GPGME is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

## Process this file with automake to produce Makefile.in

#EXTRA_DIST = gpgme-config.in gpgme.m4 mkstatus libgpgme.vers
#BUILT_SOURCES = status-table.h
#MOSTLYCLEANFILES = status-table.h
#bin_SCRIPTS = gpgme-config
#m4datadir = $(datadir)/aclocal
#m4data_DATA = gpgme.m4
#include_HEADERS = gpgme.h

#if HAVE_PTHREAD
#ltlib_gpgme_pthread = libgpgme-pthread.la
#else
#ltlib_gpgme_pthread =
#endif
#if HAVE_PTH
#ltlib_gpgme_pth = libgpgme-pth.la
#else
#ltlib_gpgme_pth =
#endif

noinst_LTLIBRARIES = libgpgme-real.la libgpgme.la # $(ltlib_gpgme_pthread) $(ltlib_gpgme_pth)

#if HAVE_LD_VERSION_SCRIPT
#libgpgme_version_script_cmd = -Wl,--version-script=$(srcdir)/libgpgme.vers
#else
#libgpgme_version_script_cmd =
#endif

#if BUILD_ASSUAN
assuan_cppflags = -I$(srcdir)/../assuan
assuan_libobjs = ../assuan/libassuan.la
#else
#assuan_cppflags =
#assuan_libobjs =
#endif

#if HAVE_DOSISH_SYSTEM
#system_components = w32-util.c w32-sema.c w32-io.c
#else
#system_components = posix-util.c posix-sema.c posix-io.c
#endif

#if HAVE_GPGSM
#gpgsm_components = engine-gpgsm.c
#else
#gpgsm_components =
#endif

libgpgme_real_la_SOURCES =						\
	conversion.c get-env.c 	\
	data.c data-fd.c data-stream.c data-mem.c data-user.c	\
	data-compat.c							\
	signers.c							\
	wait.c wait-global.c wait-private.c wait-user.c 		\
	op-support.c							\
	encrypt.c encrypt-sign.c decrypt.c decrypt-verify.c verify.c	\
	sign.c passphrase.c progress.c					\
	key.c keylist.c trust-item.c trustlist.c			\
	import.c export.c genkey.c delete.c edit.c			\
	engine.c rungpg.c 	\
	posix-util.c posix-sema.c posix-io.c	\
	engine-gpgsm.c \
	debug.c gpgme.c version.c error.c

# libgpgme_la_SOURCES = ath.h ath.c
#if HAVE_PTH
  #ath_pth_src = ath-pth-compat.c
#else
  #ath_pth_src =
#endif
#if HAVE_PTHREAD
  #ath_pthread_src = ath-pthread-compat.c
#else
  ath_pthread_src =
#endif
libgpgme_la_SOURCES = ath-compat.c # $(ath_pth_src) $(ath_pthread_src)
#libgpgme_pthread_la_SOURCES = ath.h ath-pthread.c
#libgpgme_pth_la_SOURCES = ath.h ath-pth.c

AM_CPPFLAGS = $(assuan_cppflags) -I$(srcdir)/../../libgpg-error-copy # @GPG_ERROR_CFLAGS@

#libgpgme_la_LDFLAGS = $(libgpgme_version_script_cmd) -version-info \
	#@LIBGPGME_LT_CURRENT@:@LIBGPGME_LT_REVISION@:@LIBGPGME_LT_AGE@
#libgpgme_la_DEPENDENCIES = libgpgme-real.la $(assuan_libobjs) \
	#@LTLIBOBJS@ $(srcdir)/libgpgme.vers
libgpgme_la_LIBADD = libgpgme-real.la ../../libgpg-error-copy/libgpg-error.la # $(assuan_libobjs) @LTLIBOBJS@

#libgpgme_pthread_la_LDFLAGS = $(libgpgme_version_script_cmd) -version-info \
	#@LIBGPGME_LT_CURRENT@:@LIBGPGME_LT_REVISION@:@LIBGPGME_LT_AGE@
#libgpgme_pthread_la_DEPENDENCIES = libgpgme-real.la $(assuan_libobjs) \
	#@LTLIBOBJS@ $(srcdir)/libgpgme.vers
#libgpgme_pthread_la_LIBADD = libgpgme-real.la $(assuan_libobjs) @LTLIBOBJS@ \
	#-lpthread @GPG_ERROR_LIBS@
#
#libgpgme_pth_la_CPPFLAGS = $(AM_CPPFLAGS) @PTH_CPPFLAGS@
#libgpgme_pth_la_LDFLAGS = @PTH_LDFLAGS@ \
	#$(libgpgme_version_script_cmd) -version-info \
	#@LIBGPGME_LT_CURRENT@:@LIBGPGME_LT_REVISION@:@LIBGPGME_LT_AGE@
#libgpgme_pth_la_DEPENDENCIES = libgpgme-real.la $(assuan_libobjs) \
	#@LTLIBOBJS@ $(srcdir)/libgpgme.vers
#libgpgme_pth_la_LIBADD = libgpgme-real.la $(assuan_libobjs) @LTLIBOBJS@ \
	#@PTH_LIBS@ @GPG_ERROR_LIBS@

status-table.h : $(srcdir)/gpgme.h
	$(srcdir)/mkstatus < $(srcdir)/gpgme.h > status-table.h
