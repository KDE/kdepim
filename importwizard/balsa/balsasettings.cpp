/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "balsasettings.h"

#include "mailimporter/filter_opera.h"

#include <mailtransport/transportmanager.h>
#include "mailcommon/mailutil.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>


#include <KConfig>
#include <KConfigGroup>
#include <QFile>

BalsaSettings::BalsaSettings(const QString &filename, ImportWizard *parent)
  :AbstractSettings( parent )
{
    KConfig config(filename);
    const QStringList smtpList = config.groupList().filter( QRegExp( "smtp-server-" ) );
    Q_FOREACH(const QString& smtp,smtpList) {
      KConfigGroup grp = config.group(smtp);
      readTransport(grp);
    }
    readGlobalSettings(config);
}

BalsaSettings::~BalsaSettings()
{

}

void BalsaSettings::readAccount(const KConfigGroup &grp)
{

}

void BalsaSettings::readIdentity(const KConfigGroup &grp)
{
  KPIMIdentities::Identity* newIdentity = createIdentity();
  newIdentity->setFullName(grp.readEntry(QLatin1String("FullName")));
  newIdentity->setEmailAddr(grp.readEntry(QLatin1String("Address")));
  newIdentity->setReplyToAddr(grp.readEntry(QLatin1String("ReplyTo")));
  newIdentity->setBcc(grp.readEntry(QLatin1String("Bcc")));
  const QString smtp = grp.readEntry(QLatin1String("SmtpServer"));
  if(!smtp.isEmpty() && mHashSmtp.contains(smtp)) {
    newIdentity->setTransport(mHashSmtp.value(smtp));
  }

#if 0
  Address=laurent@kspread
  ReplyTo=
  Domain=
  Bcc=
  ReplyString=Re :
  ForwardString=Fwd :
  SendMultipartAlternative=false
  SmtpServer=Défaut
  SignaturePath=
  SigExecutable=false
  SigSending=true
  SigForward=true
  SigReply=true
  SigSeparator=true
  SigPrepend=false
  FacePath=
  XFacePath=
  RequestMDN=false
  GpgSign=false
  GpgEncrypt=false
  GpgTrustAlways=false
  GpgWarnSendPlain=true
  CryptProtocol=8
  ForceKeyID=
#endif


  storeIdentity(newIdentity);
}

void BalsaSettings::readTransport(const KConfigGroup &grp)
{
  MailTransport::Transport *mt = createTransport();
  const QString smtp = grp.name().remove(QLatin1String("smtp-server-"));
  const QString server = grp.readEntry(QLatin1String("Server"));
  mt->setHost(server);

  const int tlsMode = grp.readEntry(QLatin1String("TLSMode"),-1);
  //TODO
  switch(tlsMode) {
  case 0:
      break;
  case 1:
      mt->setEncryption( MailTransport::Transport::EnumEncryption::TLS );
      break;
  case 2:
      break;
  default:
      break;
  }

  const QString ssl = grp.readEntry(QLatin1String("SSL"));
  //TODO
  if(ssl == QLatin1String("true")) {
      mt->setEncryption( MailTransport::Transport::EnumEncryption::SSL );
  } else if(ssl == QLatin1String("false")) {
      mt->setEncryption( MailTransport::Transport::EnumEncryption::None );
  } else {
      qDebug()<<" unknown ssl value :"<<ssl;
  }

  const QString anonymous = grp.readEntry(QLatin1String("Anonymous"));

  //TODO
  storeTransport( mt, /*( smtp == defaultSmtp )*/true ); //FIXME
  mHashSmtp.insert( smtp, QString::number( mt->id() ) );

    //TODO
/*
    Server=localhost:25
    Anonymous=false
    RememberPasswd=false
    SSL=false
    TLSMode=1
    BigMessage=0
*/
}

void BalsaSettings::readGlobalSettings(const KConfig &config)
{
    if(config.hasGroup(QLatin1String("Compose"))) {
        //TODO
    }
}
