/*  -*- c++ -*-
    sievejob.h

    KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "sievejob.h"
#include "sievejob_p.h"
#include "session.h"

#include <kdebug.h>
#include <kio/job.h>
#include <kio/deletejob.h>
#include <kio/jobuidelegate.h>
#include <kjobtrackerinterface.h>
#include <KUrl>

#include <QtCore/QPointer>
#include <QtCore/QTextCodec>

using KIO::Job;
using KIO::UDSEntryList;
using KIO::UDSEntry;

using namespace KManageSieve;

QHash<KUrl, QPointer<Session> > SieveJob::Private::m_sessionPool;

Session* SieveJob::Private::sessionForUrl(const KUrl& url)
{
  KUrl hostUrl( url );
  hostUrl.setPath( QString() ); // remove parts not required to identify the server
  QPointer<Session> sessionPtr = m_sessionPool.value( hostUrl );
  if ( !sessionPtr ) {
    sessionPtr = QPointer<Session>( new Session() );
    m_sessionPool.insert( hostUrl, sessionPtr );
    sessionPtr->connectToHost( hostUrl );
  }
  return sessionPtr.data();
}

void SieveJob::Private::schedule( Command command )
{
  kDebug() << command << mUrl << sessionForUrl( mUrl );
  switch ( command ) {
    case Get:
      kDebug() << "get(" << mUrl.prettyUrl() << ")";
      mJob = KIO::get( mUrl );
      q->connect( mJob, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
                  q, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
      break;
    case Put:
      kDebug() << "put(" << mUrl.prettyUrl() << ")";
      mJob = KIO::put( mUrl, 0600, KIO::Overwrite );
      q->connect( mJob, SIGNAL( dataReq( KIO::Job*, QByteArray& ) ),
                  q, SLOT( slotDataReq( KIO::Job*, QByteArray& ) ) );
      break;
    case Activate:
      kDebug() << "chmod(" << mUrl.prettyUrl() <<", 0700 )";
      mJob = KIO::chmod( mUrl, 0700 );
      break;
    case Deactivate:
      kDebug() << "chmod(" << mUrl.prettyUrl() <<", 0600 )";
      mJob = KIO::chmod( mUrl, 0600 );
      break;
    case SearchActive:
      kDebug() << "listDir(" << mUrl.prettyUrl() << ")";
      {
        KUrl url = mUrl;
        const QString query = url.query(); //save query part, because KUrl::cd() erases it
        if ( !url.fileName().isEmpty() )
          url.cd( QLatin1String( ".." ) );
        url.setQuery( query );

        kDebug() << "listDir's real URL:" << url.prettyUrl();
        mJob = KIO::listDir( url );
        q->connect( mJob, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
                    q, SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& ) ) );
        break;
      }
    case List:
      kDebug() << "listDir(" << mUrl.prettyUrl() << ")";
      {
        mJob = KIO::listDir( mUrl );
        q->connect( mJob, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
                    q, SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& ) ) );
        break;
      }
    case Delete:
      kDebug() << "delete(" << mUrl.prettyUrl() << ")";
      mJob = KIO::del( mUrl );
      break;
    default:
      Q_ASSERT( false );
  }

  // common to all jobs:
  q->connect( mJob, SIGNAL( result( KJob* ) ), q, SLOT( slotResult( KJob* ) ) );
}

static void append_lf2crlf( QByteArray & out, const QByteArray & in ) {
  if ( in.isEmpty() )
    return;
  const unsigned int oldOutSize = out.size();
  out.resize( oldOutSize + 2 * in.size() );
  const char * s = in.begin();
  const char * const end = in.end();
  char * d = out.begin() + oldOutSize;
  char last = '\0';
  while ( s < end ) {
    if ( *s == '\n' && last != '\r' )
      *d++ = '\r';
    *d++ = last = *s++;
  }
  out.resize( d - out.begin() );
}

void SieveJob::Private::run( Session *session )
{
  switch ( mCommands.top() ) {
    case Get:
    {
      const QString filename = mUrl.fileName( KUrl::ObeyTrailingSlash );
      session->sendData( "GETSCRIPT \"" + filename.toUtf8() + "\"" );
      break;
    }
    case Put:
    {
      const QString filename = mUrl.fileName( KUrl::ObeyTrailingSlash );
      QByteArray encodedData;
      append_lf2crlf( encodedData, mScript.toUtf8() );
      session->sendData( "PUTSCRIPT \"" + filename.toUtf8() + "\" {" + QByteArray::number( encodedData.size() ) + "+}" );
      session->sendData( encodedData );
      break;
    }
    case Activate:
    {
      const QString filename = mUrl.fileName( KUrl::ObeyTrailingSlash );
      session->sendData( "SETACTIVE \"" + filename.toUtf8() + "\"" );
      break;
    }
    case Deactivate:
      session->sendData( "SETACTIVE \"\"" );
      break;
    case List:
    case SearchActive:
      session->sendData( "LISTSCRIPTS" );
      break;
    case Delete:
    {
      const QString filename = mUrl.fileName( KUrl::ObeyTrailingSlash );
      session->sendData( "DELETESCRIPT \"" + filename.toUtf8() + "\"" );
      break;
    }
    default:
      Q_ASSERT( false );
  }
}

bool SieveJob::Private::handleResponse( const Response &response, const QByteArray &data )
{
  if ( response.type() == Response::Action )
    return true;
  return false;
}

void SieveJob::Private::slotData( Job*, const QByteArray &data )
{
  // check for end-of-data marker:
  if ( data.size() == 0 )
    return;

  // make sure we have a textdecoder;
  if ( !mDecoder )
    mDecoder = QTextCodec::codecForMib( 106 /*utf8*/ )->makeDecoder();

  // decode utf8; add to mScript:
  mScript += mDecoder->toUnicode( data.data(), data.size() );
}

void SieveJob::Private::slotDataReq( Job*, QByteArray &data )
{
  // check whether we have already sent our data:
  if ( mScript.isEmpty() ) {
    data = QByteArray(); // end-of-data marker
    return;
  }

  // Convert mScript into UTF-8:
  data = mScript.toUtf8();

  // "data" contains a trailing NUL, remove:
  if ( data.size() > 0 && data[(int)data.size() - 1] == '\0' )
    data.resize( data.size() - 1 );

  // mark mScript sent:
  mScript.clear();
}

void SieveJob::Private::slotEntries( Job*, const UDSEntryList &entries )
{
  foreach ( const UDSEntry &entry, entries ) {
    // Loop over all UDS atoms to find the UDSEntry::UDS_ACCESS and UDS_NAME atoms;
    // note if we find an exec'able file ( == active script )
    // or the requested filename (mUrl.fileName()).
    const QString filename = entry.stringValue( KIO::UDSEntry::UDS_NAME );

    mAvailableScripts.append( filename );
    const bool isActive = (entry.numberValue( KIO::UDSEntry::UDS_ACCESS ) == 0700);

    if ( isActive )
      mActiveScriptName = filename;

    if ( mFileExists == DontKnow && filename == mUrl.fileName() )
      mFileExists = Yes;

    emit q->item( q, filename, isActive );

    if ( mFileExists == Yes && !mActiveScriptName.isEmpty() )
      return; // early return if we have all information
  }
}

void SieveJob::Private::slotResult( KJob *job )
{
  const Command lastCmd = mCommands.top();

  // First, let's see if we come back from a SearchActive. If so, set
  // mFileExists to No if we didn't see the mUrl.fileName() during
  // listDir...
  if ( lastCmd == SearchActive && mFileExists == DontKnow && !job->error() )
    mFileExists = No;

  // prepare for next round:
  mCommands.pop();
  delete mDecoder;
  mDecoder = 0;

  // check for errors:
  if ( job->error() ) {
    if ( static_cast<KIO::Job*>(job)->ui() ) {
      static_cast<KIO::Job*>(job)->ui()->setWindow( 0 );
      static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
    }

    emit q->result( q, false, mScript, (mUrl.fileName() == mActiveScriptName) );

    if ( lastCmd == List )
      emit q->gotList( q, false, mAvailableScripts, mActiveScriptName );
    else
      emit q->gotScript( q, false, mScript, (mUrl.fileName() == mActiveScriptName) );

    mJob = 0;
    q->deleteLater();
    return;
  }

  // check for new tasks:
  if ( !mCommands.empty() ) {
    // Don't fail getting a non-existent script:
    if ( (mCommands.top() == Get) && (mFileExists == No) ) {
      mScript.clear();
      mCommands.pop();
    }
  }

  if ( mCommands.empty() ) {
    // was last command; report success and delete this object:
    emit q->result( q, true, mScript, (mUrl.fileName() == mActiveScriptName) );
    if ( lastCmd == List )
      emit q->gotList( q, true, mAvailableScripts, mActiveScriptName );
    else
      emit q->gotScript( q, true, mScript, (mUrl.fileName() == mActiveScriptName) );

    mJob = 0; // deletes itself on returning from this slot
    q->deleteLater();
    return;
  } else {
    // schedule the next command:
    schedule( mCommands.top() );
  }
}


SieveJob::SieveJob( QObject *parent )
  : QObject( parent ), d( new Private( this ) )
{
}

SieveJob::~SieveJob()
{
  kill();

  delete d;
}

void SieveJob::kill( KJob::KillVerbosity verbosity )
{
  if ( d->mJob )
    d->mJob->kill( verbosity );
}

void SieveJob::setInteractive( bool interactive )
{
  if ( d->mJob && !interactive ) {
    d->mJob->setUiDelegate( 0 );
    KIO::getJobTracker()->unregisterJob( d->mJob );
  }
}

QStringList SieveJob::sieveCapabilities() const
{
  Session* session = d->sessionForUrl( d->mUrl );
  if ( session )
    return session->sieveExtensions();
  return QStringList();
}

bool SieveJob::fileExists() const
{
  return d->mFileExists;
}

SieveJob* SieveJob::put( const KUrl &destination, const QString &script,
                         bool makeActive, bool wasActive )
{
  QStack<Private::Command> commands;
  if ( makeActive )
    commands.push( Private::Activate );

  if ( wasActive )
    commands.push( Private::Deactivate );

  commands.push( Private::Put );

  SieveJob *job = new SieveJob;
  job->d->mUrl = destination;
  job->d->mScript = script;
  job->d->mCommands = commands;
  job->d->schedule( job->d->mCommands.top() );

  Private::sessionForUrl( destination )->scheduleJob( job );
  return job;
}

SieveJob* SieveJob::get( const KUrl &source )
{
  QStack<Private::Command> commands;
  commands.push( Private::Get );
  commands.push( Private::SearchActive );

  SieveJob *job = new SieveJob;
  job->d->mUrl = source;
  job->d->mCommands = commands;
  job->d->schedule( job->d->mCommands.top() );

  Private::sessionForUrl( source )->scheduleJob( job );
  return job;
}

SieveJob* SieveJob::list( const KUrl &source )
{
  QStack<Private::Command> commands;
  commands.push( Private::List );

  SieveJob *job = new SieveJob;
  job->d->mUrl = source;
  job->d->mCommands = commands;
  job->d->schedule( job->d->mCommands.top() );

  Private::sessionForUrl( source )->scheduleJob( job );
  return job;
}

SieveJob* SieveJob::del( const KUrl &url )
{
  QStack<Private::Command> commands;
  commands.push( Private::Delete );

  SieveJob *job = new SieveJob;
  job->d->mUrl = url;
  job->d->mCommands = commands;
  job->d->schedule( job->d->mCommands.top() );

  Private::sessionForUrl( url )->scheduleJob( job );
  return job;
}

SieveJob* SieveJob::deactivate( const KUrl &url )
{
  QStack<Private::Command> commands;
  commands.push( Private::Deactivate );

  SieveJob *job = new SieveJob;
  job->d->mUrl = url;
  job->d->mCommands = commands;
  job->d->schedule( job->d->mCommands.top() );

  Private::sessionForUrl( url )->scheduleJob( job );
  return job;
}

SieveJob* SieveJob::activate( const KUrl &url )
{
  QStack<Private::Command> commands;
  commands.push( Private::Activate );

  SieveJob *job = new SieveJob;
  job->d->mUrl = url;
  job->d->mCommands = commands;
  job->d->schedule( job->d->mCommands.top() );

  Private::sessionForUrl( url )->scheduleJob( job );
  return job;
}

#include "sievejob.moc"
