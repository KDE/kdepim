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
#include "thunderbirdsettings.h"

#include "mailimporter/filter_thunderbird.h"
#include "mailimporter/filterinfo.h"
#include "mailcommon/filter/filterimporterexporter.h"
#include "importwizard.h"


#include <KLocale>
#include <KConfig>
#include <KConfigGroup>

#include <QDir>
#include <QWidget>
#include <QDebug>


ThunderbirdImportData::ThunderbirdImportData(ImportWizard*parent)
    :AbstractImporter(parent)
{
  mPath = MailImporter::FilterThunderbird::defaultPath();
}

ThunderbirdImportData::~ThunderbirdImportData()
{
}

QString ThunderbirdImportData::defaultProfile()
{
  if(mDefaultProfile.isEmpty()) {
    mDefaultProfile = MailImporter::FilterThunderbird::defaultProfile(mImportWizard);
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
  const QString accountFile = mPath + defaultProfile() + QLatin1String("/prefs.js");
  if ( QFile( accountFile ).exists() ) {
    ThunderbirdSettings account( accountFile, mImportWizard );
  } else {
    addImportSettingsInfo(i18n("Thunderbird settings not found."));
  }
  return true;
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
  const QString filterPath = mPath + defaultProfile() + QLatin1String("/Mail/Local Folders/msgFilterRules.dat");
  return addFilters( filterPath, MailCommon::FilterImporterExporter::ThunderBirdFilter );
}

AbstractImporter::TypeSupportedOptions ThunderbirdImportData::supportedOption()
{
  TypeSupportedOptions options;
  options |=AbstractImporter::Mails;
  options |=AbstractImporter::Filters;
  options |=AbstractImporter::Settings;
  return options;
}
