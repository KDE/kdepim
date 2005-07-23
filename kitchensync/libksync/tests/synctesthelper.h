/*
    This file is part of libksync.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <libkcal/calendarlocal.h>
#include <kabc/addressbook.h>
#include <kbookmarkmanager.h>

#include <qstring.h>

class TestBookmarkManager : public KBookmarkManager
{
  public:
    TestBookmarkManager() : KBookmarkManager() {}
};

class SyncTestHelper
{
  public:
    SyncTestHelper( const QString &outputDir);
    
    void sync( KCal::CalendarLocal *, KCal::CalendarLocal *,
               const QString &prefix, const QString &title );
    void sync( KABC::AddressBook *, KABC::AddressBook *,
               const QString &prefix, const QString &title );
    void sync( KBookmarkManager *, KBookmarkManager *,
               const QString &prefix, const QString &title );

    void saveAddressBook( KABC::AddressBook *ab, const QString &filename );

    void writeTestFile( const QString &prefix, const QString &type,
                        const QString &title );

  private:
    QString mOutputDir;    
};
