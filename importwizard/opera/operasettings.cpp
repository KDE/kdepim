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

#include "operasettings.h"

#include <mailtransport/transportmanager.h>
#include "mailcommon/mailutil.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>


#include <KConfig>
#include <KConfigGroup>
#include <QFile>

OperaSettings::OperaSettings(const QString &filename, ImportWizard *parent)
  :AbstractSettings( parent )
{
  if(QFile( filename ).exists()) {
    KConfig config( filename );
    KConfigGroup grp = config.group(QLatin1String("Accounts"));
    readGlobalAccount(grp);
    const QStringList accountList = config.groupList().filter( QRegExp( "Account\\d+" ) );
    const QStringList::const_iterator end( accountList.constEnd() );
    for ( QStringList::const_iterator it = accountList.constBegin(); it!=end; ++it )
    {
      KConfigGroup group = config.group( *it );
      readAccount( group );
      readTransport(group);
      readIdentity(group);
    }
  }
}

OperaSettings::~OperaSettings()
{

}

void OperaSettings::readAccount(const KConfigGroup &grp)
{
  const QString incomingProtocol = grp.readEntry(QLatin1String("Incoming Protocol"));
  QMap<QString, QVariant> settings;
  if(incomingProtocol == QLatin1String("IMAP")) {

  } else {
      qDebug()<<" protocol unknown : "<<incomingProtocol;
  }
  //TODO
}

void OperaSettings::readTransport(const KConfigGroup &grp)
{
  const QString outgoingProtocol = grp.readEntry(QLatin1String("Outgoing Protocol"));
  if(outgoingProtocol == QLatin1String("SMTP")) {
      MailTransport::Transport *mt = createTransport();
      storeTransport( mt, /*( smtp == defaultSmtp )*/true ); //FIXME:
  }
}

void OperaSettings::readIdentity(const KConfigGroup &grp)
{
    KPIMIdentities::Identity* newIdentity = createIdentity();
    //TODO
    const QString cc = grp.readEntry(QLatin1String("Auto CC"));
    const QString bcc = grp.readEntry(QLatin1String("Auto BCC"));
    const QString replyTo = grp.readEntry(QLatin1String("Replyto"));
    const QString realName = grp.readEntry(QLatin1String("Real Name"));
    const QString email = grp.readEntry(QLatin1String("Real Name"));
    const QString organization = grp.readEntry(QLatin1String("Organization"));

    storeIdentity(newIdentity);
}

void OperaSettings::readGlobalAccount(const KConfigGroup &grp)
{
  //TODO
}
