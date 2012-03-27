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
#include <kpimidentities/signature.h>

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

void SylpheedSettings::readSignature( const KConfigGroup& accountConfig, KPIMIdentities::Identity* identity )
{
  KPIMIdentities::Signature signature;
  const int signatureType = accountConfig.readEntry("signature_type", 0 );
  switch( signatureType ) {
  case 0: //File
    signature.setType( KPIMIdentities::Signature::FromFile );
    signature.setUrl( accountConfig.readEntry("signature_path" ),false );
    break;
  case 1: //Output
    signature.setType( KPIMIdentities::Signature::FromCommand );
    signature.setUrl( accountConfig.readEntry("signature_path" ),true );
    break;
  case 2: //Text
    signature.setType( KPIMIdentities::Signature::Inlined );
    signature.setText( accountConfig.readEntry("signature_text" ) );
    break;
  default:
    qDebug()<<" signature type unknow :"<<signatureType;
  }
  //TODO  const bool signatureBeforeQuote = ( accountConfig.readEntry( "signature_before_quote", 0 ) == 1 ); not implemented in kmail

  identity->setSignature( signature );
}

bool SylpheedSettings::readConfig( const QString& key, const KConfigGroup& accountConfig, QString& value )
{
  const QString useKey = QLatin1String( "set_" )+ key;
  if ( accountConfig.hasKey( useKey ) && ( accountConfig.readEntry( useKey, 0 ) == 1 ) ) {
    value = accountConfig.readEntry( key );
    return true;
  }
  return false;
}

void SylpheedSettings::readPop3Account( const KConfigGroup& accountConfig )
{
  QMap<QString, QVariant> settings;
  const QString name = accountConfig.readEntry( QLatin1String( "name" ) );
  const QString inbox = adaptFolder(accountConfig.readEntry(QLatin1String("inbox")));
  settings.insert(QLatin1String("TargetCollection"), inbox);

/*
  set_popport=0
pop_port=110
*/
  
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

  QString value;
  if ( readConfig( QLatin1String("auto_bcc") , accountConfig, value ) )
    identity->setBcc(value);

  if ( readConfig( QLatin1String("auto_cc") , accountConfig, value ) )
    identity->setReplyToAddr(value);
  
  if ( readConfig( QLatin1String("daft_folder") , accountConfig, value ) )
    identity->setDrafts(adaptFolder(value));

  if ( readConfig( QLatin1String("sent_folder") , accountConfig, value ) )
    identity->setFcc(adaptFolder(value));

  const QString transportId = readTransport(accountConfig);
  if(!transportId.isEmpty())
  {
    identity->setTransport(transportId);
  }
  readSignature( accountConfig, identity );
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
  return QString();
}
