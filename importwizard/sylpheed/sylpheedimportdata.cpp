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
#include "mailcommon/filter/filterimporterexporter.h"
#include "importwizard.h"

#include <KLocale>

#include <QDomDocument>
#include <QDomElement>
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
  QFile folderListFile( mPath + QLatin1String( "/folderlist.xml" ) );
  if ( folderListFile.exists() ) {
    QDomDocument doc;
    QString errorMsg;
    int errorRow;
    int errorCol;
    if ( !doc.setContent( &folderListFile, &errorMsg, &errorRow, &errorCol ) ) {
      kDebug() << "Unable to load document.Parse error in line " << errorRow
               << ", col " << errorCol << ": " << errorMsg;
      return QString();
    }
    QDomElement settings = doc.documentElement();

    if ( settings.isNull() ) {
      return QString();
    }

    for ( QDomElement e = settings.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
      if ( e.tagName() == QLatin1String( "folder" ) ) {
        if ( e.hasAttribute( "type" ) ) {
          if ( e.attribute( "type" ) == QLatin1String( "mh" ) ) {
            return e.attribute("path" );
          }   
        }
      }
    }
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
    const QString mailsPath = localMailDirPath();
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
  MailCommon::FilterImporterExporter importer( mImportWizard );
  bool canceled = false;
  const QString filterPath = mPath + QLatin1String("/filter.xml");
  if ( QFile( filterPath ).exists() ) {
    QList<MailCommon::MailFilter*> listFilter = importer.importFilters( canceled, MailCommon::FilterImporterExporter::SylpheedFilter, filterPath );
    appendFilters( listFilter );
    return true;
  } else {
    //TODO
  }
  
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
