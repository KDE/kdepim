/*
    This file is part of the testing framework for libkcal.

    Copyright (c) 2005 Adriaan de Groot <groot@kde.org>

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

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "calendarlocal.h"

#include <qfile.h>

using namespace KCal;

static const KCmdLineOptions options[] =
{
  {"verbose", "Verbose output", 0},
  KCmdLineLastOption
};

int main(int argc,char **argv)
{
  KAboutData aboutData("testfields","Test calendar fields read/write","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions( options );

//  KApplication app( false, false );
  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  Q_UNUSED(args)

  CalendarLocal cal( QString::fromLatin1("UTC") );

  QString file = QString::fromLatin1( INPUT );
  if (!cal.load( file ) ) {
    kdError() << "Can't load " << file << endl;
    return 1;
  }

  // 2 tests... first uid should result in a syncStatus of 0.  second uid
  // should have a new summary and a 1 for syncStatus.
  QString uid1 = QString::fromLatin1("KOrganizer-1345486115.965");
  QString uid2 = QString::fromLatin1("KOrganizer-1345486115.967");

  Event *e = cal.event( uid1 );
  if (!e) {
    kdError() << "No event " << uid1 << endl;
    return 1;
  }

  kdDebug() << "Event description " << e->summary() << endl;

  if (e->hasEndDate()) {
    QDateTime d = e->dtEnd();
    kdDebug() << "Event ends " << d << endl;
  }

  if (e->pilotId()) {
    kdDebug() << "Pilot ID = " << e->pilotId() << endl;
    kdDebug() << "Pilot Sync Status = " << e->syncStatus() << endl;
  } else {
    kdError() << "No Pilot ID" << endl;
    return 1;
  }

  kdDebug() << "First test passed.  Able to read fields." << endl;

  e->setSyncStatus(KCal::Incidence::SYNCNONE);

  QString newSummary = QString::fromLatin1("Mooo summary");

  Event *f = new Event(*e);


  f->setUid(uid2);
  // add event so we trigger updated()
  cal.addEvent(f);

  f->setPilotId(34567);
  f->setSummary(newSummary);



  QString filew = file +".out";
  // weird, yes, I know, but we have a bug right now with saving the file
  // twice which is corrupting X-PILOTSTAT
  if ( !cal.save( filew ) || !cal.save( filew ) ) {
    kdError() << "Can't save " << filew << endl;
    return 1;
  }


  // now try to read the file back in and see if our changes made it
  CalendarLocal cal2( QString::fromLatin1("UTC") );
  if (!cal2.load( filew ) ) {
    kdError() << "Can't load " << filew << endl;
    return 1;
  }

  QFile::remove( filew );

  // check for uid1--should have syncStatus of 0
  e = cal2.event( uid1 );
  if (!e) {
    kdError() << "No event for first read test" << uid1 << endl;
    return 1;
  }

  kdDebug() << "Event 1 description " << e->summary() << endl;

  if (e->pilotId()) {
    kdDebug() << "First Pilot ID = " << e->pilotId() << endl;
    kdDebug() << "First Pilot Sync Status = " << e->syncStatus() << endl;
  } else {
    kdError() << "No Pilot ID for first test" << endl;
    return 1;
  }

  if (e->syncStatus() != KCal::Incidence::SYNCNONE) {
    kdError() << "Wrong Pilot sync status." << endl;
    return 1;
  }

  // now check our second event for correctness
  f = cal2.event( uid2 );

  kdDebug() << "Event 2 description " << f->summary() << endl;

  if (f->summary() != newSummary) {
    kdError() << "Wrong summary for second read test." << endl;
    return 1;
  }

  if (f->pilotId()) {
    kdDebug() << "Second Pilot ID = " << f->pilotId() << endl;
    kdDebug() << "Second Pilot Sync Status = " << f->syncStatus() << endl;
  } else {
    kdError() << "No Pilot ID for second read test" << endl;
    return 1;
  }

  if (f->syncStatus() != KCal::Incidence::SYNCMOD) {
    kdError() << "Wrong Pilot sync status for second read test." << endl;
    return 1;
  }

  return 0;
}
