/*
 * This file is part of KMail.
 *
 * Copyright (c) 2010 KDAB
 *
 * Authors: Tobias Koenig <tokoe@kde.org>
 *         Leo Franchi    <lfranchi@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "emailaddressresolvejob.h"

#include "aliasesexpandjob.h"
#include "settings/messagecomposersettings.h"

#include "messagecomposer/composer/composer.h"
#include "messagecomposer/part/infopart.h"

#include <KPIMUtils/Email>

using namespace MessageComposer;

EmailAddressResolveJob::EmailAddressResolveJob( QObject *parent )
  : KJob( parent ), mJobCount( 0 )
{
}

EmailAddressResolveJob::~EmailAddressResolveJob()
{
}

static inline bool containsAliases( const QString &address )
{
  // an valid email is defined as foo@foo.extension
  return !(address.contains( QLatin1Char( '@' ) ) && address.contains( QLatin1Char( '.' ) ) );
}

static bool containsAliases( const QStringList &addresses )
{
  foreach ( const QString &address, addresses ) {
    if ( containsAliases( address ) )
      return true;
  }

  return false;
}

void EmailAddressResolveJob::start()
{
  QVector<AliasesExpandJob*> jobs;

  if ( containsAliases( mFrom ) ) {
    AliasesExpandJob *job = new AliasesExpandJob( mFrom, MessageComposerSettings::defaultDomain(), this );
    job->setProperty( "id", QLatin1String( "infoPartFrom" ) );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotAliasExpansionDone(KJob*)) );
    jobs << job;
  }
  if ( containsAliases( mTo ) ) {
    AliasesExpandJob *job = new AliasesExpandJob( mTo.join( QLatin1String( ", " ) ), MessageComposerSettings::defaultDomain(), this );
    job->setProperty( "id", QLatin1String( "infoPartTo" ) );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotAliasExpansionDone(KJob*)) );
    jobs << job;
  }

  if ( containsAliases( mCc ) ) {
    AliasesExpandJob *job = new AliasesExpandJob( mCc.join( QLatin1String( ", " ) ), MessageComposerSettings::defaultDomain(), this );
    job->setProperty( "id", QLatin1String( "infoPartCc" ) );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotAliasExpansionDone(KJob*)) );
    jobs << job;
  }

  if ( containsAliases( mBcc ) ) {
    AliasesExpandJob *job = new AliasesExpandJob( mBcc.join( QLatin1String( ", " ) ), MessageComposerSettings::defaultDomain(), this );
    job->setProperty( "id", QLatin1String( "infoPartBcc" ) );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotAliasExpansionDone(KJob*)) );
    jobs << job;
  }

  mJobCount = jobs.count();

  if ( mJobCount == 0 ) {
    emitResult();
  } else {
    foreach ( AliasesExpandJob *job, jobs )
      job->start();
  }
}

void EmailAddressResolveJob::slotAliasExpansionDone( KJob *job )
{
  if ( job->error() ) {
    setError( job->error() );
    setErrorText( job->errorText() );
    emitResult();
    return;
  }

  const AliasesExpandJob *expandJob = qobject_cast<AliasesExpandJob*>( job );
  mResultMap.insert( expandJob->property( "id" ).toString(), expandJob->addresses() );

  mJobCount--;
  if ( mJobCount == 0 ) {
    emitResult();
  }
}

void EmailAddressResolveJob::setFrom( const QString &from )
{
  mFrom = from;
  mResultMap.insert( QLatin1String( "infoPartFrom" ), from );
}

void EmailAddressResolveJob::setTo( const QStringList &to )
{
  mTo = to;
  mResultMap.insert( QLatin1String( "infoPartTo" ), to.join( QLatin1String( ", " ) ) );
}

void EmailAddressResolveJob::setCc( const QStringList &cc )
{
  mCc = cc;
  mResultMap.insert( QLatin1String( "infoPartCc" ), cc.join( QLatin1String( ", " ) ) );
}

void EmailAddressResolveJob::setBcc( const QStringList &bcc )
{
  mBcc = bcc;
  mResultMap.insert( QLatin1String( "infoPartBcc" ), bcc.join( QLatin1String( ", " ) ) );
}


QString EmailAddressResolveJob::expandedFrom() const
{
  return mResultMap.value( QLatin1String( "infoPartFrom" ) ).toString();
}

QStringList EmailAddressResolveJob::expandedTo() const
{
  return KPIMUtils::splitAddressList( mResultMap.value( QLatin1String( "infoPartTo" ) ).toString() );
}

QStringList EmailAddressResolveJob::expandedCc() const
{
  return KPIMUtils::splitAddressList( mResultMap.value( QLatin1String( "infoPartCc" ) ).toString() );

}

QStringList EmailAddressResolveJob::expandedBcc() const
{
  return KPIMUtils::splitAddressList( mResultMap.value( QLatin1String( "infoPartBcc" ) ).toString() );
}

#include "emailaddressresolvejob.moc"
