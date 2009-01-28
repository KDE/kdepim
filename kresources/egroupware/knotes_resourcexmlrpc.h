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

#include <QString>
#include <QList>

#include <kurl.h>

#include <kcal/calendarlocal.h>
#include <kcal/journal.h>
#include "knotes/resourcenotes.h"
#include "egroupware_export.h"

namespace KXMLRPC {
class Server;
}

class Synchronizer;

namespace KNotes {

class EGroupwarePrefs;

/**
  This class provides access to eGroupware notes via XML-RPC.
*/
class KNOTES_XMLRPC_EXPORT ResourceXMLRPC : public ResourceNotes
{
  Q_OBJECT

  public:
    ResourceXMLRPC();
    explicit ResourceXMLRPC( const KConfigGroup &group );
    virtual ~ResourceXMLRPC();

    void readConfig( const KConfigGroup &group );
    void writeConfig( KConfigGroup &group );

    EGroupwarePrefs *prefs() const { return mPrefs; }

    bool load();
    bool save();

    bool addNote( KCal::Journal* );
    bool deleteNote( KCal::Journal* );

    KCal::Alarm::List alarms( const KDateTime& from, const KDateTime& to );

  protected slots:
    void loginFinished( const QList<QVariant>&, const QVariant& );
    void logoutFinished( const QList<QVariant>&, const QVariant& );

    void listNotesFinished( const QList<QVariant>&, const QVariant& );
    void addNoteFinished( const QList<QVariant>&, const QVariant& );
    void updateNoteFinished( const QList<QVariant>&, const QVariant& );
    void deleteNoteFinished( const QList<QVariant>&, const QVariant& );

    void fault( int, const QString&, const QVariant& );

  private:
    void init();
    void initEGroupware();

    void writeNote( KCal::Journal*, QMap<QString, QVariant>& );
    void readNote( const QMap<QString, QVariant>&, KCal::Journal*, QString& );

    KCal::CalendarLocal mCalendar;
    KXMLRPC::Server *mServer;

    EGroupwarePrefs *mPrefs;

    QString mSessionID;
    QString mKp3;
    QMap<QString, QString> mUidMap;

    Synchronizer *mSynchronizer;
};

}

#endif
