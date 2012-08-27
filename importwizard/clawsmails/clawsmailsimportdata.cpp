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

#include "clawsmails/clawsmailsimportdata.h"
#include "clawsmails/clawsmailssettings.h"
#include "clawsmails/clawsmailsaddressbook.h"
#include "mailcommon/filter/filterimporter/filterimporterclawsmails_p.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"
#include "importwizard.h"

#include <KLocale>

#include <QDir>
#include <QWidget>


ClawsMailsImportData::ClawsMailsImportData(ImportWizard*parent)
  :AbstractImporter(parent)
{
  mPath = QDir::homePath() + QLatin1String("/.claws-mail/");
}

ClawsMailsImportData::~ClawsMailsImportData()
{
}


bool ClawsMailsImportData::foundMailer() const
{
  QDir directory( mPath );
  if ( directory.exists() )
    return true;
  return false;
}

QString ClawsMailsImportData::name() const
{
  return QLatin1String("Claws Mails");
}

bool ClawsMailsImportData::importMails()
{
#if 0
  MailImporter::FilterInfo *info = initializeInfo();

  MailImporter::FilterBalsa balsa;
  balsa.setFilterInfo( info );
  info->setStatusMessage(i18n("Import in progress"));
  const QString mailPath(QDir::homePath()+ QLatin1String("mail"));
  QDir directory(mailPath);
  if(directory.exists())
    balsa.importMails(mailPath);
  else
    balsa.import();
  info->setStatusMessage(i18n("Import finished"));

  delete info;
#endif
  return true;
}

bool ClawsMailsImportData::importAddressBook()
{
  const QString addressbookFile(mPath+QLatin1String("config"));
  ClawsMailsAddressBook addressbook(addressbookFile,mImportWizard);
  return true;
}

bool ClawsMailsImportData::importSettings()
{
  const QString settingFile(mPath+QLatin1String("config"));
  ClawsMailsSettings settings(settingFile,mImportWizard);
  return true;
}

bool ClawsMailsImportData::importFilters()
{
  const QString filterPath = mPath + QLatin1String("/matcherrc");
  return addFilters( filterPath, MailCommon::FilterImporterExporter::ClawsMailsFilter );
}


AbstractImporter::TypeSupportedOptions ClawsMailsImportData::supportedOption()
{
  TypeSupportedOptions options;
  options |=AbstractImporter::Mails;
  options |=AbstractImporter::AddressBooks;
  options |=AbstractImporter::Settings;
  options |=AbstractImporter::Filters;
  return options;
}
