/*
    This file is part of KAddressbook.
    Copyright (c) 2003  Alexander Kellett <lypanov@kde.org>
                        Tobias Koenig <tokoe@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef BOOKMARK_XXPORT_H
#define BOOKMARK_XXPORT_H

#include <xxport.h>

class BookmarkXXPort : public KAB::XXPort
{
  Q_OBJECT

  public:
    BookmarkXXPort( KABC::AddressBook *ab, TQWidget *parent, const char *name = 0 );

    TQString identifier() const { return "bookmark"; }

  public slots:
    bool exportContacts( const KABC::AddresseeList &list, const TQString &data );

  signals:
    /**
      The following signals are used for building a bookmarks file
      using KBookmarkDomBuilder.
     */
    void newBookmark( const TQString &text, const TQCString &url, const TQString &additionnalInfo );
    void newFolder( const TQString &text, bool open, const TQString &additionnalInfo );
    void newSeparator();
    void endFolder();
};

#endif
