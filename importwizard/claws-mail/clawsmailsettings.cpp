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

#include "clawsmailsettings.h"

#include <mailtransport/transportmanager.h>
#include "mailcommon/mailutil.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/signature.h>


#include <KConfig>
#include <KConfigGroup>
#include <QFile>

ClawsMailSettings::ClawsMailSettings(ImportWizard *parent)
  :SylpheedSettings( parent )
{
}

ClawsMailSettings::~ClawsMailSettings()
{

}

void ClawsMailSettings::importSettings(const QString& filename, const QString& path)
{
    //TODO improve it
  bool checkMailOnStartup = true;
  int intervalCheckMail = -1;
  const QString sylpheedrc = path + QLatin1String("/clawsrc");
  if(QFile( sylpheedrc ).exists()) {
    KConfig configCommon( sylpheedrc );
    if(configCommon.hasGroup("Common")) {
      KConfigGroup common = configCommon.group("Common");
      checkMailOnStartup = ( common.readEntry("check_on_startup",1) == 1 );
         if(common.readEntry(QLatin1String("autochk_newmail"),1) == 1 ) {
          intervalCheckMail = common.readEntry(QLatin1String("autochk_interval"),-1);
      }
      readGlobalSettings(common);
    }
  }
  KConfig config( filename );
  const QStringList accountList = config.groupList().filter( QRegExp( "Account: \\d+" ) );
  const QStringList::const_iterator end( accountList.constEnd() );
  for ( QStringList::const_iterator it = accountList.constBegin(); it!=end; ++it )
  {
    KConfigGroup group = config.group( *it );
    readAccount( group, checkMailOnStartup, intervalCheckMail );
    readIdentity( group );
  }
  const QString customheaderrc = path + QLatin1String("/customheaderrc");
  QFile customHeaderFile(customheaderrc);
  if(customHeaderFile.exists()) {
    if ( !customHeaderFile.open( QIODevice::ReadOnly ) ) {
      kDebug()<<" We can't open file"<<customheaderrc;
    } else {
      readCustomHeader(&customHeaderFile);
    }
  }
}
