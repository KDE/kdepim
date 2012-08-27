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

#include "clawsmailssettings.h"

#include <mailtransport/transportmanager.h>
#include "mailcommon/mailutil.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>


#include <KConfig>
#include <KConfigGroup>
#include <QFile>

ClawsMailsSettings::ClawsMailsSettings(const QString &filename, ImportWizard *parent)
  :AbstractSettings( parent )
{
  KConfig config(filename);
  const QStringList accountList = config.groupList().filter( QRegExp( "Account: \\d+" ) );
  const QStringList::const_iterator end( accountList.constEnd() );
  for ( QStringList::const_iterator it = accountList.constBegin(); it!=end; ++it )
  {
    KConfigGroup group = config.group( *it );
    //readAccount( group, checkMailOnStartup, intervalCheckMail );
    readIdentity( group );
  }
}

ClawsMailsSettings::~ClawsMailsSettings()
{

}

void ClawsMailsSettings::readAccount(const KConfigGroup &grp, bool autoCheck, int autoDelay)
{
}

void ClawsMailsSettings::readIdentity(const KConfigGroup &grp)
{
  KPIMIdentities::Identity* newIdentity = createIdentity();
  storeIdentity(newIdentity);
}

void ClawsMailsSettings::readTransport(const KConfigGroup &grp)
{
  MailTransport::Transport *mt = createTransport();
  storeTransport( mt, /*( smtp == defaultSmtp )*/true ); //FIXME
  //TODO mHashSmtp.insert( smtp, QString::number( mt->id() ) );
}

void ClawsMailsSettings::readGlobalSettings(const KConfig &config)
{
}
