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
  //TODO
  QMap<QString, QVariant> settings;
  createResource( "akonadi_pop3_resource", settings );
}

void SylpheedSettings::readImapAccount( const KConfigGroup& accountConfig )
{
  //TODO
  QMap<QString, QVariant> settings;
  createResource( "akonadi_imap_resource", settings );
}


void SylpheedSettings::readAccount( const KConfigGroup& accountConfig )
{
  if ( accountConfig.hasKey( QLatin1String( "protocol" ) ) )
  {
    const int protocol = accountConfig.readEntry( QLatin1String( "protocol" ), 0 );

    //TODO
    QMap<QString, QVariant> settings;

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
  const QString name = accountConfig.readEntry( QLatin1String( "name" ) );
  
}
  
void SylpheedSettings::readIdentity( const KConfigGroup& accountConfig )
{
  const QString organization = accountConfig.readEntry( QLatin1String( "organization" ), QString() );
  const QString email = accountConfig.readEntry( QLatin1String( "address" ) );
  const QString bcc = accountConfig.readEntry(QLatin1String("auto_bcc"));
  const QString cc = accountConfig.readEntry(QLatin1String("auto_cc"));
  const QString draft = accountConfig.readEntry(QLatin1String("draft_folder"));
  const QString sent = accountConfig.readEntry(QLatin1String("sent_folder"));
  KPIMIdentities::Identity* identity  = createIdentity();
  identity->setOrganization(organization);
  identity->setPrimaryEmailAddress(email);
  identity->setBcc(bcc);
  identity->setDrafts(draft); //FIXME
  identity->setFcc(sent);//FIXME

  //identity->setcc(cc); //FIXME
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
