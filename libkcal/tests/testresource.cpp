/* This file is part of the KDE project
   Copyright (C) 2004 Till Adam <adam@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "testresource.h"
#include "testincidencegenerator.h"

#include <config.h>

#include <kabc/stdaddressbook.h>
#include <kurl.h>
#include <kapplication.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kinputdialog.h>
#include <kresources/factory.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "calendarresources.h"
#include "resourcecalendar.h"
#include "icalformat.h"
#include "event.h"

static const KCmdLineOptions options[] =
{
  { "resource <type>", "The resource to test", 0 },
  { "configfile <file>", "Location of a config file for the resource", 0 },
  KCmdLineLastOption // End of options
};

int main(int argc, char *argv[])
{
    // Use another directory than the real one, just to keep things clean
    // KDEHOME needs to be writable though, for a ksycoca database
    setenv( "KDEHOME", QFile::encodeName( QDir::homeDirPath() + "/.kde-testresource" ), true );
    setenv( "KDE_FORK_SLAVES", "yes", true ); // simpler, for the final cleanup

    KApplication::disableAutoDcopRegistration();
    KCmdLineArgs::init(argc,argv,"testresource", 0, 0, 0, 0);
    KCmdLineArgs::addCmdLineOptions( options );
 
    KApplication app;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString type = QString::null;
    if ( args->getOption( "resource" ) )
      type = QString::fromLocal8Bit( args->getOption( "resource" ) );
    KConfig *config = 0;
    if ( args->getOption( "configfile" ) )
      config = new KConfig( KURL( args->getOption( "configfile" ) ).url() );
    kdDebug() << KURL( args->getOption( "configfile" ) ).url() << endl;
    KCal::TestResource test( type, config );
    test.setup();
    test.runAll();
    test.cleanup();
    kdDebug() << "All tests OK." << endl;
    return 0;
}

namespace KCal {

TestResource::TestResource( const QString &type, KConfig *config )
 :m_resource_type( type ), m_config( config ), m_res( 0 )
{}
  
void TestResource::setup()
{
  CalendarResourceManager *manager = new CalendarResourceManager( "calendar" );
  manager->readConfig();

  QStringList resources = manager->resourceTypeNames();

  if ( m_resource_type.isNull() ) {

    const QString & chosen = KInputDialog::getItem( "Select Resource",
        "Select the resource you wish to test. Test data will be used.", 
        resources );

    kdDebug() << "Selected Resource: " << chosen << endl;
    if ( !chosen.isNull() )
      m_resource_type = chosen;
  }
  assert( !m_resource_type.isNull() );
  /* Either read one from the config file, or create a default one. */
  if ( m_config ) {
    kdDebug() << "Reading config from file" << endl;
    KRES::Factory *factory = KRES::Factory::self( "calendar" );
    m_res = dynamic_cast<ResourceCalendar*>( factory->resource( m_resource_type, m_config ) );
  } else {
    kdDebug() << "Creating blank resource" << endl;
    m_res = manager->createResource( m_resource_type );
  }
  assert( m_res );
}


void TestResource::runAll()
{
  testOpenAndClose();
  /* now we can trust it to open correctly */
  m_res->open();
  testResourceAttributes();
  testResourceCalendarAttributes();
  testEventAddRemove();
  testTodoAddRemove();
  testJournalAddRemove();
  m_res->close();
}

bool TestResource::check(const QString& txt, QString a, QString b)
{
    if (a.isEmpty())
        a = QString::null;
    if (b.isEmpty())
        b = QString::null;
    if (a == b) {
        kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
    }
    else {
        kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
        cleanup();
        exit(1);
    }
    return true;
}

void TestResource::testOpenAndClose()
{
  kdDebug() << k_funcinfo << endl;
  assert( m_res->open() );
  assert( m_res->isOpen() );
  m_res->close();
  assert( !m_res->isOpen() );
}

void TestResource::testResourceAttributes()
{
  kdDebug() << k_funcinfo << endl;
  
  check( "type", m_res->type(), m_resource_type );
  
  m_res->setReadOnly( true );
  assert( m_res->readOnly() );
  m_res->setReadOnly( false );
  assert( !m_res->readOnly() );
 
  m_res->setResourceName( "Margarete" );
  check( "name", m_res->resourceName(), "Margarete" );

  m_res->setActive( false );
  assert( !m_res->isActive() );
  m_res->setActive( true );
  assert( m_res->isActive() );
  m_res->dump();
}

void TestResource::testResourceCalendarAttributes()
{
  kdDebug() << k_funcinfo << endl;
}


void TestResource::testEventAddRemove()
{
  ICalFormat f;
  kdDebug() << k_funcinfo << endl;
  
  int oldcount = m_res->rawIncidences().count();
  Event *event = makeTestEvent();
  const QString origString = f.toString( event );
  m_res->addEvent( event );
  Event *fromRes = m_res->event( event->uid() );
  assert( fromRes == event );
  const QString fromResString = f.toString( fromRes );
  check( "add", origString, fromResString );
  m_res->deleteEvent( event );
  assert( !m_res->event( event->uid() ) );
  int newcount = m_res->rawIncidences().count();
  assert( oldcount == newcount );
  delete event;
}

void TestResource::testTodoAddRemove()
{
  kdDebug() << k_funcinfo << endl;
}

void TestResource::testJournalAddRemove()
{
  kdDebug() << k_funcinfo << endl;
}

void TestResource::cleanup()
{
  kdDebug() << k_funcinfo << endl;
}

}





#include "testresource.moc"
