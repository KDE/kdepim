/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef WEBDAVHANDLER_H
#define WEBDAVHANDLER_H

#include <tqvaluelist.h>
#include <tqstring.h>
#include <tqdatetime.h>
#include <tqdom.h>

class WebdavHandler
{
  public:
    WebdavHandler();


    static TQDomElement addElement( TQDomDocument &, TQDomNode &,
                                   const TQString &tag );
    static TQDomElement addDavElement( TQDomDocument &, TQDomNode &,
                                      const TQString &tag );
    static TQDomElement addSloxElement( TQDomDocument &, TQDomNode &,
                                       const TQString &tag,
                                       const TQString &text = TQString::null );
    static TQDomDocument createAllPropsRequest();
};

#endif
