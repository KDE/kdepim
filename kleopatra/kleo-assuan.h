/* -*- mode: c++; c-basic-offset:4 -*-
    kleo-assuan.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEO_ASSUAN_H__
#define __KLEO_ASSUAN_H__

#ifndef _ASSUAN_ONLY_GPG_ERRORS
#define _ASSUAN_ONLY_GPG_ERRORS
#endif

#ifdef HAVE_USABLE_ASSUAN
# include <assuan.h>
#else
/*
 * copied from assuan.h:
 */

/* assuan.h - Definitions for the Assuan IPC library
 * Copyright (C) 2001, 2002, 2003, 2005, 2007 Free Software Foundation, Inc.
 *
 * This file is part of Assuan.
 *
 * Assuan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Assuan is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef _WIN32
typedef void *assuan_fd_t;
#define ASSUAN_INVALID_FD ((void*)(-1))
#define ASSUAN_INT2FD(s)  ((void *)(s))
#define ASSUAN_FD2INT(h)  ((unsigned int)(h))
#else
typedef int assuan_fd_t;
#define ASSUAN_INVALID_FD (-1)
#define ASSUAN_INT2FD(s)  ((s))
#define ASSUAN_FD2INT(h)  ((h))
#endif
/*
 * end copied from assuan.h
 */
#endif

#endif /* __KLEOPATRA_ASSUAN_H__ */
