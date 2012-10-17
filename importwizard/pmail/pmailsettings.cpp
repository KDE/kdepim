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

#include "pmailsettings.h"
#include "importwizardutil.h"

#include <mailtransport/transportmanager.h>
#include "mailcommon/mailutil.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>


#include <KConfig>
#include <KConfigGroup>
#include <QFile>

PMailSettings::PMailSettings(const QString& filename,ImportWizard *parent)
  :AbstractSettings( parent )
{
  if(QFile( filename ).exists()) {
    KConfig config( filename );

    const QStringList accountList = config.groupList().filter( QRegExp( "WinPMail Identity - *" ) );
    const QStringList::const_iterator end( accountList.constEnd() );
    for ( QStringList::const_iterator it = accountList.constBegin(); it!=end; ++it )
    {
      KConfigGroup group = config.group( *it );
      readIdentity( group );
    }
  }
}

PMailSettings::~PMailSettings()
{

}

void PMailSettings::readIdentity( const KConfigGroup& group )
{
    KPIMIdentities::Identity* newIdentity = createIdentity();
    const QString personalNameStr = QLatin1String("Personal name                             ");
    if(group.hasKey(personalNameStr)) {
        const QString personalName = group.readEntry(personalNameStr);
        newIdentity->setFullName( personalName );
        newIdentity->setIdentityName( personalName );
    }
    const QString emailStr = QLatin1String("Internet E-mail Address                   ");
    if(group.hasKey(emailStr)) {
        const QString email = group.readEntry(emailStr);
        newIdentity->setPrimaryEmailAddress(email);
    }
    storeIdentity(newIdentity);
}
