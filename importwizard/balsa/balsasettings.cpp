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
}

BalsaSettings::~BalsaSettings()
{

}

void BalsaSettings::readAccount(const KConfigGroup &grp)
{

}

void BalsaSettings::readIdentity(const KConfigGroup &grp)
{

}

void BalsaSettings::readTransport(const KConfigGroup &grp)
{
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

void BalsaSettings::readGlobalSettings(const KConfigGroup &grp)
{

}
