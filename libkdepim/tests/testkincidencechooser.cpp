/*
  Copyright (C) 2009 Allen Winter <winter@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include <kdebug.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <libkcal/event.h>
using namespace KCal;

#include "kincidencechooser.h"
using namespace KPIM;

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "testkincidencechooser", 0,
                      "KIncidenceChooserTest", "1.0",
                      "kincidencechooser test app" );
  KApplication app;
  QString folder( "My Calendar" );
  KIncidenceChooser *chooser = new KIncidenceChooser( folder, KIncidenceChooser::Sync, false, 0 );

  Event ev1;
  ev1.setSummary(
    i18n( "This is a very long summary of a meeting we will be having soon. "
          "This will help us test how the dialog works with long incidence "
          "summaries and the like." ) );
  ev1.setDescription( i18n( "Discuss foo" ) );

  Event ev2;
  ev2.setSummary( i18n( "meeting" ) );
  ev2.setDescription( i18n( "Let's have a big meeting where we discuss nothing at all" ) );

  chooser->setIncidences( &ev1, &ev2 );
  chooser->show();

  if ( chooser->exec() ) {
    kdDebug() << "User selected ask policy=" << chooser->conflictAskPolicy() << endl;
    kdDebug() << "User take=" << chooser->takeMode() << endl;
    kdDebug() << "Folder Only?=" << chooser->folderOnly() << endl;
  }

  return 0;
}
