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

#include "sylpheed/sylpheedimportdata.h"
#include "mailimporter/filter_sylpheed.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"

#include <KLocale>

#include <QDir>
#include <QWidget>


SylpheedImportData::SylpheedImportData(ImportWizard*parent)
    :PimImportAbstract(parent)
{
    mPath = QDir::homePath() + QLatin1String( "/.sylpheed-2.0/" );
}

SylpheedImportData::~SylpheedImportData()
{
}

QString SylpheedImportData::localMailDirPath()
{
  QFile folderlist( mPath + QLatin1String( "/folderlist.xml" ) );
  if ( folderlist.exists() ) {
    //TODO
  }
  return QString();
}

bool SylpheedImportData::foundMailer() const
{
  QDir directory( mPath );
  if ( directory.exists() )
    return true;
  return false;
}

QString SylpheedImportData::name() const
{
  return QLatin1String("Sylpheed");
}

bool SylpheedImportData::importSettings()
{
  return false;
}

bool SylpheedImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();

    info->clear(); // Clear info from last time
 
    MailImporter::FilterSylpheed sylpheed;
    sylpheed.setFilterInfo( info );
    info->setStatusMessage(i18n("Import in progress"));
    const QString mailsPath = mPath  + localMailDirPath();
    QDir directory(mailsPath);
    if(directory.exists())
        sylpheed.importMails(mailsPath);
    else
        sylpheed.import();
    info->setStatusMessage(i18n("Import finished"));

    delete info;
    return true;
}

bool SylpheedImportData::importFilters()
{
  return false;
}

bool SylpheedImportData::importAddressBook()
{
  return false;
}

PimImportAbstract::TypeSupportedOptions SylpheedImportData::supportedOption()
{
  TypeSupportedOptions options;
  options |=PimImportAbstract::Mails;
  options |=PimImportAbstract::Filters;
  return options;
}
