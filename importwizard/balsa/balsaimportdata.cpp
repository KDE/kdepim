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

#include "balsa/balsaimportdata.h"
#include "balsa/balsasettings.h"
#include "balsa/balsaaddressbook.h"
#include "mailimporter/filterbalsa.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"
#include "importwizard.h"

#include <KLocale>

#include <QDir>
#include <QWidget>


BalsaImportData::BalsaImportData(ImportWizard*parent)
  :AbstractImporter(parent)
{
  mPath = QDir::homePath() + QLatin1String("/.balsa/");
}

BalsaImportData::~BalsaImportData()
{
}


bool BalsaImportData::foundMailer() const
{
  QDir directory( mPath );
  if ( directory.exists() )
    return true;
  return false;
}

QString BalsaImportData::name() const
{
  return QLatin1String("Balsa");
}

bool BalsaImportData::importMails()
{
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
  return true;
}

bool BalsaImportData::importAddressBook()
{
  const QString addressbookFile(mPath+QLatin1String("config"));
  BalsaAddressBook addressbook(addressbookFile,mImportWizard);
  return true;
}

bool BalsaImportData::importSettings()
{
  const QString settingFile(mPath+QLatin1String("config"));
  BalsaSettings settings(settingFile,mImportWizard);
  return true;
}

AbstractImporter::TypeSupportedOptions BalsaImportData::supportedOption()
{
  TypeSupportedOptions options;
  options |=AbstractImporter::Mails;
  options |=AbstractImporter::AddressBooks;
  options |=AbstractImporter::Settings;
  return options;
}
