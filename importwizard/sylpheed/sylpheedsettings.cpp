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

#include "sylpheedsettings.h"
#include <mailtransport/transportmanager.h>

#include <kpimidentities/identity.h>

#include <KConfig>
#include <KConfigGroup>

#include <QRegExp>
#include <QStringList>

SylpheedSettings::SylpheedSettings( const QString& filename, ImportWizard *parent )
    :AbstractSettings( parent )
{
  KConfig config( filename );
  const QStringList accountList = config.groupList().filter( QRegExp( "Account:\\d+" ) );
  const QStringList::const_iterator end( accountList.constEnd() );
  for ( QStringList::const_iterator it = accountList.constBegin(); it!=end; ++it )
  {
    KConfigGroup group = config.group( *it );
    readAccount( group );
    readIdentity( group );
  }
}

SylpheedSettings::~SylpheedSettings()
{
}
   
void SylpheedSettings::readPop3Account( const KConfigGroup& accountConfig )
{
  QMap<QString, QVariant> settings;
  const QString name = accountConfig.readEntry( QLatin1String( "name" ) );
  const QString inbox = adaptFolder(accountConfig.readEntry(QLatin1String("inbox")));
  settings.insert(QLatin1String("TargetCollection"), inbox);
  createResource( "akonadi_pop3_resource", name, settings );
}

void SylpheedSettings::readImapAccount( const KConfigGroup& accountConfig )
{
  //TODO
  QMap<QString, QVariant> settings;
  const QString name = accountConfig.readEntry( QLatin1String( "name" ) );

  createResource( "akonadi_imap_resource", name,settings );
}


void SylpheedSettings::readAccount( const KConfigGroup& accountConfig )
{
  if ( accountConfig.hasKey( QLatin1String( "protocol" ) ) )
  {
    const int protocol = accountConfig.readEntry( QLatin1String( "protocol" ), 0 );
    switch( protocol )
    {
      case 0:
        readPop3Account( accountConfig );
        break;
      case 3:
        //imap
        readImapAccount(accountConfig);
        break;
      case 4:
        qDebug()<<" we can't create news item";
        //news
        break;
      case 5:
        //local
        break;
    }
  }  
}
  
void SylpheedSettings::readIdentity( const KConfigGroup& accountConfig )
{
  KPIMIdentities::Identity* identity  = createIdentity();
  const QString organization = accountConfig.readEntry( QLatin1String( "organization" ), QString() );
  identity->setOrganization(organization);
  const QString email = accountConfig.readEntry( QLatin1String( "address" ) );
  identity->setPrimaryEmailAddress(email);
  
  if(accountConfig.readEntry(QLatin1String("set_autobcc"),0)==1 ) {
    const QString bcc = accountConfig.readEntry(QLatin1String("auto_bcc"));
    identity->setBcc(bcc);
  }

  const QString cc = accountConfig.readEntry(QLatin1String("auto_cc"));
  //identity->setcc(cc); //FIXME
  const QString draft = accountConfig.readEntry(QLatin1String("draft_folder"));
  identity->setDrafts(adaptFolder(draft));

  if(accountConfig.readEntry(QLatin1String("set_sent_folder"),0) == 1) {
    const QString sent = accountConfig.readEntry(QLatin1String("sent_folder"));
    identity->setFcc(adaptFolder(sent));
  }

  const QString transportId = readTransport(accountConfig);
  if(!transportId.isEmpty())
  {
    identity->setTransport(transportId);
  }
  storeIdentity(identity);
}
  
QString SylpheedSettings::readTransport( const KConfigGroup& accountConfig )
{
  const QString smtpservername = accountConfig.readEntry("receive_server");
  const QString smtpserver = accountConfig.readEntry("smtp_server");
  if(!smtpserver.isEmpty()) {
    MailTransport::Transport *mt = createTransport();
    mt->setName( smtpservername );
    mt->writeConfig();
    MailTransport::TransportManager::self()->addTransport( mt );
    MailTransport::TransportManager::self()->setDefaultTransport( mt->id() );
    return QString::number(mt->id()); //TODO verify
    /*
  smtp_auth_method=0
  smtp_user_id=
  smtp_password=
  ssl_smtp=0
*/
  }
  return QString();//TODO
}
