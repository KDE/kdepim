/*******************************************************************
 This file is part of KNotes.

 Copyright (c) 2004, Bo Thorsen <bo@sonofthor.dk>
               2004-2006, Michael Brade <brade@kde.org>

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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA  02110-1301, USA.

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
*******************************************************************/

#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include <libkcal/icalformat.h>

#include "knotes/resourcelocal.h"
#include "knotes/resourcelocalconfig.h"
#include "knotes/resourcemanager.h"
#include "knotes/resourcenotes.h"



ResourceLocal::ResourceLocal( const KConfig *config )
    : ResourceNotes( config ), mCalendar( TQString::fromLatin1( "UTC" ) )
{
    kdDebug(5500) << "ResourceLocal::ResourceLocal()" << endl;
    setType( "file" );
    mURL = KGlobal::dirs()->saveLocation( "data", "knotes/" ) + "notes.ics";

    if ( config )
    {
        KURL u = config->readPathEntry( "NotesURL" );
        if ( !u.isEmpty() )
            mURL = u;
    }
}

ResourceLocal::~ResourceLocal()
{
}

void ResourceLocal::writeConfig( KConfig *config )
{
    KRES::Resource::writeConfig( config );
    config->writePathEntry( "NotesURL", mURL.prettyURL() );
}

bool ResourceLocal::load()
{
    mCalendar.load( mURL.path() );

    KCal::Journal::List notes = mCalendar.journals();
    KCal::Journal::List::ConstIterator it;
    for ( it = notes.constBegin(); it != notes.constEnd(); ++it )
        manager()->registerNote( this, *it );

    return true;
}

bool ResourceLocal::save()
{
    if ( !mCalendar.save( mURL.path(), new KCal::ICalFormat() ) )
    {
        KMessageBox::error( 0,
                            i18n("<qt>Unable to save the notes to <b>%1</b>. "
                                 "Check that there is sufficient disk space."
                                 "<br>There should be a backup in the same directory "
                                 "though.</qt>").arg( mURL.path() ) );
        return false;
    }

    return true;
}

bool ResourceLocal::addNote( KCal::Journal *journal )
{
    return mCalendar.addJournal( journal );
}

bool ResourceLocal::deleteNote( KCal::Journal *journal )
{
    return mCalendar.deleteJournal( journal );
}

KCal::Alarm::List ResourceLocal::alarms( const TQDateTime& from, const TQDateTime& to )
{
    KCal::Alarm::List alarms;
    KCal::Journal::List notes = mCalendar.journals();
    KCal::Journal::List::ConstIterator note;
    for ( note = notes.constBegin(); note != notes.constEnd(); ++note )
    {
        TQDateTime preTime = from.addSecs( -1 );
        KCal::Alarm::List::ConstIterator it;
        for( it = (*note)->alarms().constBegin(); it != (*note)->alarms().constEnd(); ++it )
        {
            if ( (*it)->enabled() )
            {
                TQDateTime dt = (*it)->nextRepetition( preTime );
                if ( dt.isValid() && dt <= to )
                    alarms.append( *it );
            }
        }
    }

    return alarms;
}
