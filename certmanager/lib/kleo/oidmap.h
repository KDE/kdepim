/*  -*- mode: C++; c-file-style: "gnu" -*-
    oidmap.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifndef __KLEO_OIDMAP_H__
#define __KLEO_OIDMAP_H__

static struct {
  const char * name;
  const char * oid;
} oidmap[] = {
  // keep them ordered by oid:
  { "SP", "ST" }, // hack to show the Sphinx-required/desired SP for
		  // StateOrProvince, otherwise known as ST or even S
  { "EMAIL", "1.2.840.113549.1.9.1" },
  { "SN", "2.5.4.4" },
  { "BC", "2.5.4.15" },
  { "GN", "2.5.4.42" }
};
static const unsigned int numOidMaps = sizeof oidmap / sizeof *oidmap;

#endif // __KLEO_OIDMAP_H__
