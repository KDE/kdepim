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
#include <KUrl>
#include <QtCore/QPointer>

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
  const Command lastCmd = mCommands.top();

  // handle non-action responses
  if ( response.type() != Response::Action ) {
    switch ( lastCmd ) {
      case Get:
        mScript = QString::fromUtf8( data );
        break;
      case List:
      case SearchActive:
      {
        const QString filename = QString::fromUtf8( response.key() );
        mAvailableScripts.append( filename );
        const bool isActive = response.extra() == "ACTIVE";

        if ( isActive )
          mActiveScriptName = filename;

        if ( mFileExists == DontKnow && filename == mUrl.fileName() )
          mFileExists = Yes;

        emit q->item( q, filename, isActive );
        break;
      }
      default:
        kDebug() << "Unhandled response: " << response.key() << response.value() << response.extra() << data;
    }
    return false; // we expect more
  }

  Q_ASSERT( response.type() == Response::Action );
  // First, let's see if we come back from a SearchActive. If so, set
  // mFileExists to No if we didn't see the mUrl.fileName() during
  // listDir...
  if ( lastCmd == SearchActive && mFileExists == DontKnow && response.operationSuccessful() )
    mFileExists = No;

  // prepare for next round:
  mCommands.pop();

  // check for errors:
  if ( !response.operationSuccessful() ) {
    if ( mInteractive ) {
      // TODO
//       static_cast<KIO::Job*>(job)->ui()->setWindow( 0 );
//       static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
    }

    emit q->result( q, false, mScript, (mUrl.fileName() == mActiveScriptName) );

    if ( lastCmd == List )
      emit q->gotList( q, false, mAvailableScripts, mActiveScriptName );
    else
      emit q->gotScript( q, false, mScript, (mUrl.fileName() == mActiveScriptName) );

    q->deleteLater();
    return true;
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

    q->deleteLater();
    return true;
  } else {
    // schedule the next command:
    run( sessionForUrl( mUrl ) );
    return false;
  }

  return true;
}

void SieveJob::Private::killed()
{
  emit q->result( q, false, mScript, (mUrl.fileName() == mActiveScriptName) );
  if ( mCommands.top() == List )
    emit q->gotList( q, false, mAvailableScripts, mActiveScriptName );
  else
    emit q->gotScript( q, false, mScript, (mUrl.fileName() == mActiveScriptName) );
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
  Q_UNUSED( verbosity );
  if ( d->mCommands.isEmpty() )
    return; // done already
  Private::sessionForUrl( d->mUrl )->killJob( this );
}

void SieveJob::setInteractive( bool interactive )
{
  d->mInteractive = interactive;
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

  Private::sessionForUrl( url )->scheduleJob( job );
  return job;
}

#include "sievejob.moc"
