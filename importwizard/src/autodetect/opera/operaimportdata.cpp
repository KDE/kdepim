/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "opera/operaimportdata.h"
#include "opera/operaaddressbook.h"
#include "opera/operasettings.h"
#include "mailimporter/filteropera.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"
#include "importwizard.h"

#include <KLocalizedString>

#include <QDir>

OperaImportData::OperaImportData(ImportWizard *parent)
    : AbstractImporter(parent)
{
    mPath = MailImporter::FilterOpera::defaultSettingsPath();
}

OperaImportData::~OperaImportData()
{
}

bool OperaImportData::foundMailer() const
{
    QDir directory(mPath);
    if (directory.exists()) {
        return true;
    }
    return false;
}

QString OperaImportData::name() const
{
    return QStringLiteral("Opera");
}

bool OperaImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();

    MailImporter::FilterOpera opera;
    opera.setFilterInfo(info);
    info->setStatusMessage(i18n("Import in progress"));
    const QString mailPath(mPath + QLatin1String("mail/store/"));
    QDir directory(mailPath);
    if (directory.exists()) {
        opera.importMails(mailPath);
    } else {
        opera.import();
    }
    info->setStatusMessage(i18n("Import finished"));

    delete info;
    return true;
}

bool OperaImportData::importAddressBook()
{
    const QString addressbookFile(mPath + QLatin1String("bookmarks.adr"));
    OperaAddressBook addressbook(addressbookFile, mImportWizard);
    return true;
}

bool OperaImportData::importSettings()
{
    const QString settingFile(mPath + QLatin1String("mail/accounts.ini"));
    OperaSettings settings(settingFile, mImportWizard);
    return true;
}

AbstractImporter::TypeSupportedOptions OperaImportData::supportedOption()
{
    TypeSupportedOptions options;
    options |= AbstractImporter::Mails;
    options |= AbstractImporter::AddressBooks;
    options |= AbstractImporter::Settings;
    return options;
}
