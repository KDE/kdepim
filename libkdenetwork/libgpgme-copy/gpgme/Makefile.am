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

noinst_LTLIBRARIES = libgpgme-real.la libgpgme.la

assuan_cppflags = -I$(srcdir)/../assuan
assuan_libobjs = ../assuan/libassuan.la

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

libgpgme_la_SOURCES = ath-compat.c fakes.c

AM_CPPFLAGS = $(assuan_cppflags) -I$(srcdir)/../../libgpg-error-copy

libgpgme_la_LIBADD = libgpgme-real.la ../../libgpg-error-copy/libgpg-error.la $(assuan_libobjs)

status-table.h : $(srcdir)/gpgme.h
	$(srcdir)/mkstatus < $(srcdir)/gpgme.h > status-table.h
