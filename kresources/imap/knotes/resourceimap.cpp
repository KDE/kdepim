/*
    This file is part of the IMAP resources.
    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

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

#include "resourceimap.h"

#include <resourcemanager.h>

#include <libkcal/icalformat.h>

#include <kdebug.h>
#include <kglobal.h>


class IMAPFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new KNotesIMAP::ResourceIMAP( config );
    }

    KRES::ConfigWidget *configWidget( QWidget* )
    {
      return 0;
    }
};

extern "C"
{
  void *init_knotes_imap()
  {
    return ( new IMAPFactory() );
  }
}


KNotesIMAP::ResourceIMAP::ResourceIMAP( const KConfig *config )
  : ResourceNotes( config ),
    ResourceIMAPBase::ResourceIMAPShared( "ResourceIMAP-KNotes" ),
    mSilent( false )
{
}

KNotesIMAP::ResourceIMAP::~ResourceIMAP()
{
}

bool KNotesIMAP::ResourceIMAP::load()
{
  // Get the list of journals
  QStringList lst;
  if( !kmailIncidences( lst, "Notes" ) ) {
    kdError() << "Communication problem in ResourceIMAP::getIncidenceList()\n";
    return false;
  }

  // We got a fresh list of events, so clean out the old ones
  mCalendar.deleteAllEvents();

  // Populate the calendar with the new events
  for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    KCal::Journal* journal = parseJournal( *it );
    if( journal ) {
      mCalendar.addJournal( journal );
      journal->registerObserver( this );
      manager()->registerNote( this, journal, true );
    }
  }

  return true;
}

bool KNotesIMAP::ResourceIMAP::save()
{
  kdDebug() << "NYI: KNotesIMAP::ResourceIMAP::save()\n";
  return false;
}

bool KNotesIMAP::ResourceIMAP::addNote( KCal::Journal* )
{
  kdDebug() << "NYI: KNotesIMAP::ResourceIMAP::addNote( KCal::Journal* )\n";
  return false;
}

bool KNotesIMAP::ResourceIMAP::deleteNote( KCal::Journal* )
{
  kdDebug() << "NYI: KNotesIMAP::ResourceIMAP::deleteNote( KCal::Journal* )\n";
  return false;
}

void KNotesIMAP::ResourceIMAP::incidenceUpdated( KCal::IncidenceBase* )
{
  kdDebug() << "NYI: KNotesIMAP::ResourceIMAP::incidenceUpdated( KCal::IncidenceBase* )\n";
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool KNotesIMAP::ResourceIMAP::addIncidence( const QString& type,
                                             const QString& note )
{
  Q_UNUSED( type );
  Q_UNUSED( note );
  kdDebug() << "NYI: KNotesIMAP::ResourceIMAP::addIncidence()\n";
  return false;
}

void KNotesIMAP::ResourceIMAP::deleteIncidence( const QString& type,
                                                const QString& uid )
{
  Q_UNUSED( type );
  Q_UNUSED( uid );
  kdDebug() << "NYI: KNotesIMAP::ResourceIMAP::deleteIncidence()\n";
}

void KNotesIMAP::ResourceIMAP::slotRefresh( const QString& type )
{
  Q_UNUSED( type );
  kdDebug() << "NYI: KNotesIMAP::ResourceIMAP::slotRefresh()\n";
}


KCal::Journal* KNotesIMAP::ResourceIMAP::parseJournal( const QString& str )
{
  KCal::Incidence* i = mFormat->fromString( str );
  if ( i ) {
    if ( i->type() == "Journal" )
      return static_cast<KCal::Journal*>( i );
    else {
      kdDebug() << "Unknown incidence type " << i->type() << endl;
      delete i;
    }
  } else
    kdDebug() << "Parse error\n";

  return 0;
}



#include "resourceimap.moc"
