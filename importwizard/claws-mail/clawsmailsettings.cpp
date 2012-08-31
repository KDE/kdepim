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

void ClawsMailSettings::readSettingsColor(const KConfigGroup& group)
{
  const bool enableColor = group.readEntry("enable_color", false);
  if(enableColor) {
    const QString colorLevel1 = group.readEntry("quote_level1_color");
    if(!colorLevel1.isEmpty()) {
        const QColor col = QColor(colorLevel1);
        if(col.isValid()) {
          addKmailConfig(QLatin1String("Reader"), QLatin1String("QuotedText1"), writeColor(col));
        }
      //[Reader]  QuotedText1
    }
    const QString colorLevel2 = group.readEntry("quote_level2_color");
    if(!colorLevel2.isEmpty()) {
        const QColor col = QColor(colorLevel2);
        if(col.isValid()) {
          addKmailConfig(QLatin1String("Reader"), QLatin1String("QuotedText2"), writeColor(col));
        }
      //[Reader]  QuotedText2
    }
    const QString colorLevel3 = group.readEntry("quote_level3_color");
    if(!colorLevel3.isEmpty()) {
        const QColor col = QColor(colorLevel3);
        if(col.isValid()) {
          addKmailConfig(QLatin1String("Reader"), QLatin1String("QuotedText3"), writeColor(col));
        }
      //[Reader]  QuotedText3
    }
    const QString misspellColor = group.readEntry(QLatin1String("misspelled_color"));
    if(!misspellColor.isEmpty()) {
        const QColor col = QColor(misspellColor);
        if(col.isValid()) {
          addKmailConfig(QLatin1String("Reader"), QLatin1String("MisspelledColor"), writeColor(col));
        }
    }
  }
}

QString ClawsMailSettings::writeColor(const QColor& col)
{
    QStringList list;
    list.insert(0, QString::number(col.red()));
    list.insert(1, QString::number(col.green()));
    list.insert(2, QString::number(col.blue()));
    if (col.alpha() != 255)
        list.insert(3, QString::number(col.alpha()));
    return list.join(QLatin1String(","));
}

void ClawsMailSettings::readTemplateFormat(const KConfigGroup& group)
{
  SylpheedSettings::readTemplateFormat(group);
  const QString composerNewMessage = group.readEntry(QLatin1String("compose_body_format"));
  if(!composerNewMessage.isEmpty()) {
    addKmailConfig(QLatin1String("TemplateParser"), QLatin1String("TemplateNewMessage"), convertToKmailTemplate(composerNewMessage));
  }
}
