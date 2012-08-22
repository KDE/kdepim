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

    bool autoCheck = false;
    int autoCheckDelay = -1;
    if(config.hasGroup(QLatin1String("MailboxChecking"))) {
      KConfigGroup grp = config.group(QLatin1String("MailboxChecking"));
      autoCheck = grp.readEntry(QLatin1String("Auto"),false);
      autoCheckDelay = grp.readEntry(QLatin1String("AutoDelay"),-1);
    }

    const QStringList mailBoxList = config.groupList().filter( QRegExp( "mailbox-\\+d" ) );
    Q_FOREACH(const QString& mailBox,mailBoxList) {
      KConfigGroup grp = config.group(mailBox);
      readAccount(grp,autoCheck,autoCheckDelay);
    }

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

void BalsaSettings::readAccount(const KConfigGroup &grp, bool autoCheck, int autoDelay)
{
  const QString type = grp.readEntry(QLatin1String("Type"));
  if(type == QLatin1String("LibBalsaMailboxPOP3")) {
      //TODO
  } else if(type == QLatin1String("LibBalsaMailboxImap")) {
      //TODO
  } else {
      qDebug()<<" unknown account type :"<<type;
  }
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
    KConfigGroup compose = config.group(QLatin1String("Compose"));
    if(compose.hasKey(QLatin1String("QuoteString"))) {
      const QString quote = compose.readEntry(QLatin1String("QuoteString"));
      if(!quote.isEmpty())
        addKmailConfig( QLatin1String("TemplateParser"), QLatin1String("QuoteString"), quote);
    }
  }
  if(config.hasGroup(QLatin1String("MessageDisplay"))) {
    KConfigGroup messageDisplay = config.group(QLatin1String("MessageDisplay"));
    if(messageDisplay.hasKey(QLatin1String("WordWrap"))) {
      bool wordWrap = messageDisplay.readEntry(QLatin1String("WordWrap"),false);
      //TODO not implemented in kmail.
    }
    if(messageDisplay.hasKey(QLatin1String("WordWrapLength"))) {
      const int wordWrapLength = messageDisplay.readEntry(QLatin1String("WordWrapLength"),-1);
      //TODO not implemented in kmail
    }
  }

  if(config.hasGroup(QLatin1String("Sending"))) {
    KConfigGroup sending = config.group(QLatin1String("Sending"));
    if(sending.hasKey(QLatin1String("WordWrap"))) {
       const bool wordWrap = sending.readEntry(QLatin1String("WordWrap"),false);
       addKmailConfig( QLatin1String("Composer"), QLatin1String("word-wrap"), wordWrap);
    }
    if(sending.hasKey(QLatin1String("break-at"))) {
      const int wordWrapLength = sending.readEntry(QLatin1String("break-at"),-1);
      if(wordWrapLength!=-1) {
        addKmailConfig( QLatin1String("Composer"), QLatin1String("break-at"),wordWrapLength);
      }
    }
  }
  if(config.hasGroup(QLatin1String("Global"))) {
    KConfigGroup global = config.group(QLatin1String("Global"));
    if(global.hasKey(QLatin1String("EmptyTrash"))) {
      const bool emptyTrash = global.readEntry(QLatin1String("EmptyTrash"),false);
      addKmailConfig( QLatin1String("General"), QLatin1String("empty-trash-on-exit"),emptyTrash);
    }
  }
}
