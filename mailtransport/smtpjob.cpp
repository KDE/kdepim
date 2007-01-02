/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "smtpjob.h"
#include "transport.h"
#include "mailtransport_defs.h"

#include <klocale.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/passdlg.h>

#include <qbuffer.h>

using namespace KPIM;

SmtpJob::SmtpJob(Transport * transport, QObject * parent) :
    TransportJob( transport, parent )
{
}

void SmtpJob::start()
{
  QString query = "headers=0&from=";
  query += KUrl::toPercentEncoding( sender() );

  foreach ( QString str, to() )
    query += "&to=" + KUrl::toPercentEncoding( str );
  foreach ( QString str, cc() )
    query += "&cc=" + KUrl::toPercentEncoding( str );
  foreach ( QString str, bcc() )
    query += "&bcc=" + KUrl::toPercentEncoding( str );

  if ( transport()->specifyHostname() )
    query += "&hostname=" + KUrl::toPercentEncoding( transport()->localHostname() );

#warning Argh!
//   if ( !kmkernel->msgSender()->sendQuotedPrintable() )
//     query += "&body=8bit";

  KUrl destination;

  destination.setProtocol( (transport()->encryption() == Transport::EnumEncryption::SSL) ? SMTPS_PROTOCOL : SMTP_PROTOCOL );
  destination.setHost( transport()->host() );
  destination.setPort( transport()->port().toUShort() );

  if ( transport()->requiresAuthentication() ) {
    if( (transport()->userName().isEmpty() || transport()->password().isEmpty())
         && transport()->authenticationType() != Transport::EnumAuthenticationType::GSSAPI )
    {
      QString user = transport()->userName();
      QString passwd = transport()->password();
      bool keep = transport()->storePassword();
      int result;

#warning yet another KMail specific thing
//       KCursorSaver idle( KBusyPtr::idle() );
      result = KIO::PasswordDialog::getNameAndPassword(
          user, passwd, &keep,
          i18n("You need to supply a username and a password to use this SMTP server."),
          false, QString(), transport()->name(), QString() );

      if ( result != QDialog::Accepted ) {
        setError( KilledJobError );
        emitResult();
        return;
      }
      transport()->setUserName( user );
      transport()->setPassword( passwd );
      transport()->setStorePassword( keep );
      transport()->writeConfig();
    }
    destination.setUser( transport()->userName() );
    destination.setPass( transport()->password() );
  }

// TODO: from kmsender.cpp
//   if (!mSlave || !mInProcess)
//   {
//     KIO::MetaData slaveConfig;
//     slaveConfig.insert("tls", (ti->encryption == "TLS") ? "on" : "off");
//     if (ti->auth) slaveConfig.insert("sasl", ti->authType);
//     mSlave = KIO::Scheduler::getConnectedSlave(destination, slaveConfig);
//   }
//
//   if (!mSlave)
//   {
//     abort();
//     return false;
//   }

  // dotstuffing is now done by the slave (see setting of metadata)
  if ( !data().isEmpty() )
    // allow +5% for subsequent LF->CRLF and dotstuffing (an average
    // over 2G-lines gives an average line length of 42-43):
    query += "&size=" + QString::number( qRound( data().length() * 1.05 ) );

  destination.setPath("/send");
  destination.setQuery( query );
  kDebug() << k_funcinfo << destination << endl;

  KIO::Job *job = KIO::put( destination, -1, false, false, false );
  Q_ASSERT( job );
  job->addMetaData( "lf2crlf+dotstuff", "slave" );
  connect( job, SIGNAL(dataReq(KIO::Job*,QByteArray&)), SLOT(dataRequest(KIO::Job*,QByteArray&)) );

  addSubjob( job );
  // TODO from kmsender.cpp
/*  KIO::Scheduler::assignJobToSlave(mSlave, mJob);
  connect(mJob, SIGNAL(result(KJob *)), this, SLOT(result(KJob *))); */
}

void SmtpJob::dataRequest(KIO::Job * job, QByteArray & data)
{
  Q_ASSERT( job );
  if ( buffer()->atEnd() )
    data.clear();
  else
    data = buffer()->read( 32*1024 );
  // TODO: from kmsender.cpp
//   mSender->emitProgressInfo( mMessageOffset );
}

#include "smtpjob.moc"
