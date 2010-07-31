 /*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KNOTES_RESOURCEXMLRPC_H
#define KNOTES_RESOURCEXMLRPC_H

#include <tqptrlist.h>
#include <tqstring.h>

#include <kconfig.h>
#include <kurl.h>
#include <kdepimmacros.h>

#include "libkcal/calendarlocal.h"
#include "libkcal/journal.h"
#include "knotes/resourcenotes.h"

namespace KXMLRPC {
class Server;
}

class Synchronizer;

namespace KNotes {

class EGroupwarePrefs;

/**
  This class provides access to eGroupware notes via XML-RPC.
*/
class KDE_EXPORT ResourceXMLRPC : public ResourceNotes
{
  Q_OBJECT

  public:
    ResourceXMLRPC( const KConfig* );
    ResourceXMLRPC();
    virtual ~ResourceXMLRPC();

    void readConfig( const KConfig* config );
    void writeConfig( KConfig* config );

    EGroupwarePrefs *prefs() const { return mPrefs; }

    bool load();
    bool save();

    bool addNote( KCal::Journal* );
    bool deleteNote( KCal::Journal* );

    KCal::Alarm::List alarms( const TQDateTime& from, const TQDateTime& to );

  protected slots:
    void loginFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void logoutFinished( const TQValueList<TQVariant>&, const TQVariant& );

    void listNotesFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void addNoteFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void updateNoteFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void deleteNoteFinished( const TQValueList<TQVariant>&, const TQVariant& );

    void fault( int, const TQString&, const TQVariant& );

  private:
    void init();
    void initEGroupware();

    void writeNote( KCal::Journal*, TQMap<TQString, TQVariant>& );
    void readNote( const TQMap<TQString, TQVariant>&, KCal::Journal*, TQString& );

    KCal::CalendarLocal mCalendar;
    KXMLRPC::Server *mServer;

    EGroupwarePrefs *mPrefs;

    TQString mSessionID;
    TQString mKp3;
    TQMap<TQString, TQString> mUidMap;

    Synchronizer *mSynchronizer;
};

}

#endif
