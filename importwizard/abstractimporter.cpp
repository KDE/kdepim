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

#include "abstractimporter.h"
#include "importwizard.h"
#include "importmailpage.h"
#include "importfilterinfogui.h"
#include "importfilterpage.h"

#include "mailimporter/filterinfo.h"
#include "mailcommon/filter/filtermanager.h"

#include <QFile>

AbstractImporter::AbstractImporter(ImportWizard *parent)
    :mImportWizard(parent)
{
}

AbstractImporter::~AbstractImporter()
{
}


bool AbstractImporter::importSettings()
{
  return false;
}

bool AbstractImporter::importMails()
{
  return false;
}

bool AbstractImporter::importFilters()
{
  return false;
}

bool AbstractImporter::importAddressBook()
{
  return false;
}

MailImporter::FilterInfo* AbstractImporter::initializeInfo()
{
    MailImporter::FilterInfo *info = new MailImporter::FilterInfo();
    ImportFilterInfoGui *infoGui = new ImportFilterInfoGui(mImportWizard->importMailPage());
    info->setFilterInfoGui(infoGui);
    info->setRootCollection( mImportWizard->importMailPage()->selectedCollection() );
    return info;
}

bool AbstractImporter::addFilters( const QString& filterPath, MailCommon::FilterImporterExporter::FilterType type )
{
  if ( QFile( filterPath ).exists() ) {
    bool canceled = false;
    MailCommon::FilterImporterExporter importer( mImportWizard );
    QList<MailCommon::MailFilter*> listFilter = importer.importFilters( canceled, type, filterPath );
    appendFilters( listFilter );
    addFilterImportInfo( i18np( "1 filter was imported", "%1 filters were imported", listFilter.count() ) );
    return true;
  } else {
    addFilterImportError( i18n( "Filters file was not found" ) );
    return false;
  }
}

void AbstractImporter::appendFilters( const QList<MailCommon::MailFilter*>& filters )
{
  if ( !filters.isEmpty() )
    MailCommon::FilterManager::instance()->appendFilters(filters, false );
}

void AbstractImporter::addFilterImportInfo( const QString& log )
{
  mImportWizard->importFilterPage()->addFilterImportInfo( log );
}

void AbstractImporter::addFilterImportError( const QString& log )
{
  mImportWizard->importFilterPage()->addFilterImportError( log );
}
