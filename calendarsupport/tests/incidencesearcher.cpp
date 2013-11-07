/*
    Copyright (c) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    Author: Sérgio Martins <sergio.martins@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "incidencesearcher.h"

#include "../next/incidencesearchjob.h"

#include <KCalCore/Incidence>

#include <kcmdlineargs.h>
#include <kapplication.h>

#include <QDebug>

using namespace CalendarSupport;
using namespace Akonadi;

IncidenceSearcher::IncidenceSearcher( const QString &uid )
{
  // Exact Match
  IncidenceSearchJob *job1 = new IncidenceSearchJob();
  job1->setQuery( CalendarSupport::IncidenceSearchJob::IncidenceUid, uid,
                  IncidenceSearchJob::ExactMatch );
  connect( job1, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)) );

  // StartsWithMatch
  IncidenceSearchJob *job2 = new IncidenceSearchJob();
  job2->setQuery( IncidenceSearchJob::IncidenceUid, uid,
                 IncidenceSearchJob::StartsWithMatch );
  connect( job2, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)) );

  // ContainsMatch
  IncidenceSearchJob *job3 = new IncidenceSearchJob();
  job3->setQuery( IncidenceSearchJob::IncidenceUid, uid,
                  IncidenceSearchJob::ContainsMatch );
  connect( job3, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)) );

  // No query
  IncidenceSearchJob *job4 = new IncidenceSearchJob();
  connect( job4, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)) );

  mJobs << job1 << job2 << job3 << job4;
}

void IncidenceSearcher::finished( KJob *job )
{
  static int count = 0;
  ++count;

  Q_ASSERT( !job->error() );

 IncidenceSearchJob *searchJob = qobject_cast<IncidenceSearchJob*>( job );

 const KCalCore::Incidence::List incidences = searchJob->incidences();
 QStringList matches;
 matches << QLatin1String( "ExactMatch" )
         << QLatin1String( "StartsWithMatch" )
         << QLatin1String( "ContainsMatch" )
         << QLatin1String( "No query" );

 qDebug() << "Found " << incidences.count() << " incidences using " <<
   matches[mJobs.indexOf( job )];

 if ( !incidences.isEmpty() ) {
   qDebug() << "Incidences are:";
   foreach( const KCalCore::Incidence::Ptr &incidence, incidences ) {
     if ( incidence ) {
       qDebug() << "Summary = " << incidence->summary() << "; uid = "
                << incidence->uid() << "; schedulingId = " << incidence->schedulingID();
     }
   }
 }


  if ( count == 4 ) {
    qDebug() << "Finished";
    qApp->quit();
  }
}

int main( int argc, char* argv[] )
{
  KCmdLineOptions options;
  options.add( "+uid", ki18n( "Incidence uid to search for" ) );
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs::init( argc, argv, "IncidenceSearchJob", 0,
                      ki18n("IncidenceSearcher") ,"0.1" ,ki18n("IncidenceSearchJob demo") );

  KApplication app( false );
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  IncidenceSearcher is( args->arg( 0 ) );

  return app.exec();
}

