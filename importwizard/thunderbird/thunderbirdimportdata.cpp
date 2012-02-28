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

#include "thunderbirdimportdata.h"
#include "importfilterinfogui.h"
#include "importwizard.h"

#include "mailimporter/filter_thunderbird.h"
#include "mailimporter/filterinfo.h"

#include "mailcommon/filter/filterimporterexporter.h"

#include <KLocale>
#include <KConfig>
#include <KConfigGroup>

#include <QDir>
#include <QWidget>
#include <QDebug>


ThunderbirdImportData::ThunderbirdImportData(ImportWizard*parent)
    :PimImportAbstract(parent)
{
  mPath = MailImporter::FilterThunderbird::defaultPath();
}

ThunderbirdImportData::~ThunderbirdImportData()
{
}

QString ThunderbirdImportData::defaultProfile()
{
    if(mDefaultProfile.isEmpty()) {
      QFile profiles( mPath + QLatin1String( "/profiles.ini" ) );
      if ( profiles.exists() ) {
        //ini file.
        KConfig config( mPath + QLatin1String( "/profiles.ini" ) );
        //TODO look at support of multi profile
        if ( config.hasGroup( "Profile0" ) ) {
          KConfigGroup group = config.group( "Profile0" );
          const QString path = group.readEntry( "Path" );
          return path;
        }
      }
        
    }
    return mDefaultProfile;
}

bool ThunderbirdImportData::foundMailer() const
{
  QDir directory( mPath );
  if ( directory.exists() )
    return true;
  return false;
}

QString ThunderbirdImportData::name() const
{
  return QLatin1String("Thunderbird");
}

bool ThunderbirdImportData::importSettings()
{
  return false;
}

bool ThunderbirdImportData::importMails()
{
    //* This should be usually ~/.thunderbird/xxxx.default/Mail/Local Folders/
    MailImporter::FilterInfo *info = initializeInfo();


    MailImporter::FilterThunderbird thunderbird;
    thunderbird.setFilterInfo( info );
    info->clear();
    info->setStatusMessage(i18n("Import in progress"));
    const QString mailsPath = mPath + defaultProfile() + QLatin1String("/Mail/Local Folders/");
    QDir directory(mailsPath);
    if(directory.exists())
        thunderbird.importMails(mailsPath);
    else
        thunderbird.import();
    info->setStatusMessage(i18n("Import finished"));

    delete info;
    return true;
}

bool ThunderbirdImportData::importFilters()
{
  MailCommon::FilterImporterExporter importer( mImportWizard );
  bool canceled = false;
  const QString filterPath = mPath + defaultProfile() + QLatin1String("/Mail/Local Folders/msgFilterRules.dat");
  if ( QFile( filterPath ).exists() ) {
    QList<MailCommon::MailFilter*> listFilter = importer.importFilters( canceled, MailCommon::FilterImporterExporter::ThunderBirdFilter, filterPath );
  } else {
    //TODO
  }
  
  return false;
}

bool ThunderbirdImportData::importAddressBook()
{
  return false;
}

PimImportAbstract::TypeSupportedOptions ThunderbirdImportData::supportedOption()
{
  TypeSupportedOptions options;
  options |=PimImportAbstract::Mails;
  options |=PimImportAbstract::Filters;
  return options;
}
