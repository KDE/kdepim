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
#include "messagecomposersettings.h"

#include "messagecomposer/composer.h"
#include "messagecomposer/infopart.h"

#include <KPIMUtils/Email>

using namespace MessageComposer;

EmailAddressResolveJob::EmailAddressResolveJob( QObject *parent )
  : KJob( parent ), mJobCount( 0 )
{
}

EmailAddressResolveJob::~EmailAddressResolveJob()
{
}

void EmailAddressResolveJob::start()
{
  // Increment this immediatley, as an AliasesExpandJob might finish immediatley,
  // for empty addresses
  mJobCount = 4;

  {
    AliasesExpandJob *job = new AliasesExpandJob( mFrom, MessageComposerSettings::defaultDomain(), this );
    job->setProperty( "id", QLatin1String("infoPartFrom") );
    connect( job, SIGNAL( result( KJob* ) ), SLOT( slotAliasExpansionDone( KJob* ) ) );
    job->start();
  }
/*  {
    if ( !mMessage->bcc()->addresses().isEmpty() ) {
      QStringList bcc;
      foreach( const QByteArray &address, mMessage->bcc()->addresses() )
        bcc << QString::fromUtf8( address );

      AliasesExpandJob *job = new AliasesExpandJob( bcc.join( QLatin1String( ", " ) ), GlobalSettings::defaultDomain(), this );
      job->setProperty( "id", "messageBcc" );
      connect( job, SIGNAL( result( KJob* ) ), SLOT( slotAliasExpansionDone( KJob* ) ) );
      job->start();

      mJobCount++;
    }
  } */
  {
    AliasesExpandJob *job = new AliasesExpandJob( mTo.join( QLatin1String( ", " ) ), MessageComposerSettings::defaultDomain(), this );
    job->setProperty( "id", QLatin1String("infoPartTo") );
    connect( job, SIGNAL( result( KJob* ) ), SLOT( slotAliasExpansionDone( KJob* ) ) );
    job->start();
  }
  {
    AliasesExpandJob *job = new AliasesExpandJob( mCc.join( QLatin1String( ", " ) ), MessageComposerSettings::defaultDomain(), this );
    job->setProperty( "id", QLatin1String("infoPartCc") );
    connect( job, SIGNAL( result( KJob* ) ), SLOT( slotAliasExpansionDone( KJob* ) ) );
    job->start();
  }
  {
    AliasesExpandJob *job = new AliasesExpandJob( mBcc.join( QLatin1String( ", " ) ), MessageComposerSettings::defaultDomain(), this );
    job->setProperty( "id", QLatin1String("infoPartBcc") );
    connect( job, SIGNAL( result( KJob* ) ), SLOT( slotAliasExpansionDone( KJob* ) ) );
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

void EmailAddressResolveJob::setFrom(const QString& from)
{
  mFrom = from;
}

void EmailAddressResolveJob::setTo(const QStringList& to)
{
  mTo = to;
}

void EmailAddressResolveJob::setCc(const QStringList& cc)
{
  mCc = cc;
}

void EmailAddressResolveJob::setBcc(const QStringList& bcc)
{
  mBcc = bcc;
}


QString EmailAddressResolveJob::expandedFrom() const
{
  return mResultMap.value( QLatin1String("infoPartFrom") ).toString();
}

QStringList EmailAddressResolveJob::expandedTo() const
{
  return KPIMUtils::splitAddressList( mResultMap.value( QLatin1String("infoPartTo") ).toString() );
}

QStringList EmailAddressResolveJob::expandedCc() const
{
  return KPIMUtils::splitAddressList( mResultMap.value( QLatin1String("infoPartCc") ).toString() );

}

QStringList EmailAddressResolveJob::expandedBcc() const
{
  return KPIMUtils::splitAddressList( mResultMap.value( QLatin1String("infoPartBcc") ).toString() );

}

#include "emailaddressresolvejob.moc"
