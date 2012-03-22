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
   

void SylpheedSettings::readAccount( const KConfigGroup& accountConfig )
{
  if ( accountConfig.hasKey( QLatin1String( "protocol" ) ) )
  {
    const int protocol = accountConfig.readEntry( QLatin1String( "protocol" ), 0 );
    switch( protocol )
    {
    case 0:
      //pop3
      break;
    case 3:
      //imap
      break;
    case 4:
      //news
      break;
    case 5:
      //local
      break;
    }
  }
  const QString name = accountConfig.readEntry( QLatin1String( "name" ) );
  const QString smtp = accountConfig.readEntry( QLatin1String( "smtp_server" ) );
  
}
  
void SylpheedSettings::readIdentity( const KConfigGroup& accountConfig )
{
  const QString organization = accountConfig.readEntry( QLatin1String( "organization" ), QString() );
  const QString email = accountConfig.readEntry( QLatin1String( "address" ) );
  KPIMIdentities::Identity* identity  = createIdentity();
  identity->setOrganization(organization);
  identity->setPrimaryEmailAddress(email);
  storeIdentity(identity);
}
  
