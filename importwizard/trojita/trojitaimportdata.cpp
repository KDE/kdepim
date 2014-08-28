/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "trojita/trojitaimportdata.h"
#include "trojita/trojitasettings.h"
#include "trojita/trojitaaddressbook.h"
#include "mailimporter/filterinfo.h"
#include "mailimporter/othermailerutil.h"
#include "importfilterinfogui.h"
#include "importwizard.h"

#include <QDir>

TrojitaImportData::TrojitaImportData(ImportWizard *parent)
    : AbstractImporter(parent)
{
    mPath = MailImporter::OtherMailerUtil::trojitaDefaultPath();
}

TrojitaImportData::~TrojitaImportData()
{
}

bool TrojitaImportData::foundMailer() const
{
    QDir directory(mPath);
    if (directory.exists()) {
        return true;
    }
    return false;
}

QString TrojitaImportData::name() const
{
    return QLatin1String("Trojita");
}

bool TrojitaImportData::importMails()
{
    return false;
}

bool TrojitaImportData::importSettings()
{
    const QString settingsPath = mPath + QLatin1String("trojita.conf");
    TrojitaSettings settings(settingsPath, mImportWizard);
    return true;
}

bool TrojitaImportData::importAddressBook()
{
    const QString addressbookPath = QDir::homePath() + QLatin1String("/.abook/addressbook");
    TrojitaAddressBook addressBooks(addressbookPath, mImportWizard);
    addressBooks.readAddressBook();
    return true;
}

AbstractImporter::TypeSupportedOptions TrojitaImportData::supportedOption()
{
    TypeSupportedOptions options;
    //options |=AbstractImporter::Mails;
    options |= AbstractImporter::Settings;
    options |= AbstractImporter::AddressBooks;
    return options;
}
