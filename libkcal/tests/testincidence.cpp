/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
*/

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "icalformat.h"
#include "event.h"
#include "todo.h"

using namespace KCal;

static const KCmdLineOptions options[] =
{
  {"verbose", "Verbose output", 0},
  KCmdLineLastOption
};

int main(int argc,char **argv)
{
  KAboutData aboutData("testincidence","Test Incidence","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions( options );

//  KApplication app( false, false );
  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  bool verbose = false;
  if ( args->isSet( "verbose" ) ) verbose = true;

  ICalFormat f;

  Event *event1 = new Event;
  event1->setSummary("Test Event");
  event1->recurrence()->setDaily( 2, 3 );

  QString eventString1 = f.toString( event1 );
  if ( verbose )
    kdDebug() << "EVENT1 START:" << eventString1 << "EVENT1 END" << endl;

  Incidence *event2 = event1->clone();

  QString eventString2 = f.toString( event2 );
  if( verbose )
    kdDebug() << "EVENT2 START:" << eventString2 << "EVENT2 END" << endl;

  if ( eventString1 != eventString2 ) {
    kdDebug() << "Clone Event FAILED." << endl;
  } else {
    kdDebug() << "Clone Event SUCCEEDED." << endl;
  }

  Todo *todo1 = new Todo;
  todo1->setSummary("Test todo");
  QString todoString1 = f.toString( todo1 );
  if( verbose )
    kdDebug() << "todo1 START:" << todoString1 << "todo1 END" << endl;
    
  Incidence *todo2 = todo1->clone();
  QString todoString2 = f.toString( todo2 );
  if( verbose )
    kdDebug() << "todo2 START:" << todoString2 << "todo2 END" << endl;

  if ( todoString1 != todoString2 ) {
    kdDebug() << "Clone Todo FAILED." << endl;
  } else {
    kdDebug() << "Clone Todo SUCCEEDED." << endl;
  }
}
