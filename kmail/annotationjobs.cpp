/**
 * annotationjobs.cpp
 *
 * Copyright (c) 2004 David Faure <faure@kde.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */
#include "annotationjobs.h"
#include <kio/scheduler.h>
#include <kdebug.h>

using namespace KMail;

KIO::SimpleJob* AnnotationJobs::setAnnotation(
    KIO::Slave* slave, const KURL& url, const TQString& entry,
    const TQMap<TQString,TQString>& attributes )
{
  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int)'M' << (int)'S' << url << entry << attributes;

  KIO::SimpleJob* job = KIO::special( url, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

AnnotationJobs::GetAnnotationJob* AnnotationJobs::getAnnotation(
    KIO::Slave* slave, const KURL& url, const TQString& entry,
    const TQStringList& attributes )
{
  TQByteArray packedArgs;
  TQDataStream stream( packedArgs, IO_WriteOnly );
  stream << (int)'M' << (int)'G' << url << entry << attributes;

  GetAnnotationJob* job = new GetAnnotationJob( url, entry, packedArgs, false );
  KIO::Scheduler::assignJobToSlave( slave, job );
  return job;
}

AnnotationJobs::GetAnnotationJob::GetAnnotationJob( const KURL& url, const TQString& entry,
                                                    const TQByteArray &packedArgs,
                                                    bool showProgressInfo )
  : KIO::SimpleJob( url, KIO::CMD_SPECIAL, packedArgs, showProgressInfo ),
    mEntry( entry )
{
  connect( this, TQT_SIGNAL(infoMessage(KIO::Job*,const TQString&)),
           TQT_SLOT(slotInfoMessage(KIO::Job*,const TQString&)) );
}

void AnnotationJobs::GetAnnotationJob::slotInfoMessage( KIO::Job*, const TQString& str )
{
  // Parse the result
  TQStringList lst = TQStringList::split( "\r", str );
  while ( lst.count() >= 2 ) // we take items 2 by 2
  {
    TQString name = lst.front(); lst.pop_front();
    TQString value = lst.front(); lst.pop_front();
    mAnnotations.append( AnnotationAttribute( mEntry, name, value ) );
  }
}

AnnotationJobs::MultiGetAnnotationJob::MultiGetAnnotationJob(
  KIO::Slave* slave, const KURL& url, const TQStringList& entries, bool showProgressInfo )
  : KIO::Job( showProgressInfo ),
    mSlave( slave ),
    mUrl( url ), mEntryList( entries ), mEntryListIterator( mEntryList.begin() )
{
  TQTimer::singleShot(0, this, TQT_SLOT(slotStart()));
}


void AnnotationJobs::MultiGetAnnotationJob::slotStart()
{
  if ( mEntryListIterator != mEntryList.end() ) {
    TQStringList attributes;
    attributes << "value";
    KIO::Job* job = getAnnotation( mSlave, mUrl, *mEntryListIterator, attributes );
    addSubjob( job );
  } else { // done!
    emitResult();
  }
}

void AnnotationJobs::MultiGetAnnotationJob::slotResult( KIO::Job *job )
{
  if ( job->error() ) {
    KIO::Job::slotResult( job ); // will set the error and emit result(this)
    return;
  }
  subjobs.remove( job );
  const TQString& entry = *mEntryListIterator;
  TQString value;
  bool found = false;
  GetAnnotationJob* getJob = static_cast<GetAnnotationJob *>( job );
  const AnnotationList& lst = getJob->annotations();
  for ( unsigned int i = 0 ; i < lst.size() ; ++ i ) {
    //kdDebug(5006) << "found annotation " << lst[i].name << " = " << lst[i].value << endl;
    if ( lst[i].name.startsWith( "value." ) ) { // value.priv or value.shared
      found = true;
      value = lst[i].value;
      break;
    }
  }
  emit annotationResult( entry, value, found );
  // Move on to next one
  ++mEntryListIterator;
  slotStart();
}

AnnotationJobs::MultiGetAnnotationJob* AnnotationJobs::multiGetAnnotation( KIO::Slave* slave, const KURL& url, const TQStringList& entries )
{
  return new MultiGetAnnotationJob( slave, url, entries, false /*showProgressInfo*/ );
}

////

AnnotationJobs::MultiSetAnnotationJob::MultiSetAnnotationJob(
  KIO::Slave* slave, const KURL& url, const AnnotationList& annotations, bool showProgressInfo )
  : KIO::Job( showProgressInfo ),
    mSlave( slave ),
    mUrl( url ), mAnnotationList( annotations ), mAnnotationListIterator( mAnnotationList.begin() )
{
  TQTimer::singleShot(0, this, TQT_SLOT(slotStart()));
}


void AnnotationJobs::MultiSetAnnotationJob::slotStart()
{
  if ( mAnnotationListIterator != mAnnotationList.end() ) {
    const AnnotationAttribute& attr = *mAnnotationListIterator;
    // setAnnotation can set multiple attributes for a given entry.
    // So in theory we could group entries coming from our list. Bah.
    TQMap<TQString, TQString> attributes;
    attributes.insert( attr.name, attr.value );
    kdDebug() << k_funcinfo << " setting annotation " << attr.entry << " " << attr.name << " " << attr.value << endl;
    KIO::Job* job = setAnnotation( mSlave, mUrl, attr.entry, attributes );
    addSubjob( job );
  } else { // done!
    emitResult();
  }
}

void AnnotationJobs::MultiSetAnnotationJob::slotResult( KIO::Job *job )
{
  if ( job->error() ) {
    KIO::Job::slotResult( job ); // will set the error and emit result(this)
    return;
  }
  subjobs.remove( job );
  const AnnotationAttribute& attr = *mAnnotationListIterator;
  emit annotationChanged( attr.entry, attr.name, attr.value );

  // Move on to next one
  ++mAnnotationListIterator;
  slotStart();
}

AnnotationJobs::MultiSetAnnotationJob* AnnotationJobs::multiSetAnnotation(
  KIO::Slave* slave, const KURL& url, const AnnotationList& annotations )
{
  return new MultiSetAnnotationJob( slave, url, annotations, false /*showProgressInfo*/ );
}


AnnotationJobs::MultiUrlGetAnnotationJob::MultiUrlGetAnnotationJob( KIO::Slave* slave,
                                                                    const KURL& baseUrl,
                                                                    const TQStringList& paths,
                                                                    const TQString& annotation )
  : KIO::Job( false ),
    mSlave( slave ),
    mUrl( baseUrl ),
    mPathList( paths ),
    mPathListIterator( mPathList.begin() ),
    mAnnotation( annotation )
{
  TQTimer::singleShot(0, this, TQT_SLOT(slotStart()));
}


void AnnotationJobs::MultiUrlGetAnnotationJob::slotStart()
{
  if ( mPathListIterator != mPathList.end() ) {
    TQStringList attributes;
    attributes << "value";
    KURL url(mUrl);
    url.setPath( *mPathListIterator );
    KIO::Job* job = getAnnotation( mSlave, url, mAnnotation, attributes );
    addSubjob( job );
  } else { // done!
    emitResult();
  }
}

void AnnotationJobs::MultiUrlGetAnnotationJob::slotResult( KIO::Job *job )
{
  if ( job->error() ) {
    KIO::Job::slotResult( job ); // will set the error and emit result(this)
    return;
  }
  subjobs.remove( job );
  const TQString& path = *mPathListIterator;
  GetAnnotationJob* getJob = static_cast<GetAnnotationJob *>( job );
  const AnnotationList& lst = getJob->annotations();
  for ( unsigned int i = 0 ; i < lst.size() ; ++ i ) {
    kdDebug(5006) << "MultiURL: found annotation " << lst[i].name << " = " << lst[i].value << " for path: " << path << endl;
    if ( lst[i].name.startsWith( "value." ) ) { // value.priv or value.shared
      mAnnotations.insert( path, lst[i].value );
      break;
    }
  }
  // Move on to next one
  ++mPathListIterator;
  slotStart();
}

TQMap<TQString, TQString> AnnotationJobs::MultiUrlGetAnnotationJob::annotations() const
{
  return mAnnotations;
}

AnnotationJobs::MultiUrlGetAnnotationJob* AnnotationJobs::multiUrlGetAnnotation( KIO::Slave* slave,
                                                                                 const KURL& baseUrl,
                                                                                 const TQStringList& paths,
                                                                                 const TQString& annotation )
{
  return new MultiUrlGetAnnotationJob( slave, baseUrl, paths, annotation );
}


#include "annotationjobs.moc"
