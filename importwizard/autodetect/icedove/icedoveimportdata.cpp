/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "icedoveimportdata.h"
#include "importfilterinfogui.h"
#include "thunderbird/thunderbirdsettings.h"
#include "thunderbird/thunderbirdaddressbook.h"

#include "mailimporter/filtericedove.h"
#include "mailimporter/filterinfo.h"
#include "MailCommon/FilterImporterExporter"
#include "importwizard.h"

#include <KLocalizedString>
#include <KConfig>

#include <QDir>

IcedoveImportData::IcedoveImportData(ImportWizard *parent)
    : AbstractImporter(parent)
{
    mPath = MailImporter::FilterIcedove::defaultSettingsPath();
}

IcedoveImportData::~IcedoveImportData()
{
}

QString IcedoveImportData::defaultProfile()
{
    if (mDefaultProfile.isEmpty()) {
        mDefaultProfile = MailImporter::FilterThunderbird::defaultProfile(mPath, mImportWizard);
    }
    return mDefaultProfile;
}

bool IcedoveImportData::foundMailer() const
{
    QDir directory(mPath);
    if (directory.exists()) {
        return true;
    }
    return false;
}

bool IcedoveImportData::importAddressBook()
{
    const QDir addressbookDir(mPath + defaultProfile());
    ThunderBirdAddressBook account(addressbookDir, mImportWizard);
    return true;
}

QString IcedoveImportData::name() const
{
    return QStringLiteral("Icedove");
}

bool IcedoveImportData::importSettings()
{
    const QString accountFile = mPath + defaultProfile() + QLatin1String("/prefs.js");
    if (QFile(accountFile).exists()) {
        ThunderbirdSettings account(accountFile, mImportWizard);
    } else {
        addImportSettingsInfo(i18n("Thunderbird settings not found."));
    }
    return true;
}

bool IcedoveImportData::importMails()
{
    //* This should be usually ~/.icedove/xxxx.default/Mail/Local Folders/
    MailImporter::FilterInfo *info = initializeInfo();

    MailImporter::FilterThunderbird thunderbird;
    thunderbird.setFilterInfo(info);
    info->clear();
    info->setStatusMessage(i18n("Import in progress"));
    const QString mailsPath = mPath + defaultProfile() + QLatin1String("/Mail/Local Folders/");
    QDir directory(mailsPath);
    if (directory.exists()) {
        thunderbird.importMails(mailsPath);
    } else {
        thunderbird.import();
    }
    info->setStatusMessage(i18n("Import finished"));

    delete info;
    return true;
}

bool IcedoveImportData::importFilters()
{
    const QString path(mPath + defaultProfile());
    QDir dir(path);
    bool filtersAdded = false;
    const QStringList subDir = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
    if (subDir.isEmpty()) {
        return true;
    }

    Q_FOREACH (const QString &mailPath, subDir) {
        const QString subMailPath(path + QLatin1Char('/') + mailPath);
        QDir dirMail(subMailPath);
        const QStringList subDirMail = dirMail.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
        bool foundFilterFile = false;
        Q_FOREACH (const QString &file, subDirMail) {
            const QString filterFile(subMailPath + QLatin1Char('/') + file + QLatin1String("/msgFilterRules.dat"));
            if (QFile(filterFile).exists()) {
                foundFilterFile = true;
                const bool added = addFilters(filterFile, MailCommon::FilterImporterExporter::ThunderBirdFilter);
                if (!filtersAdded && added) {
                    filtersAdded = true;
                }
            }
        }
        if (!foundFilterFile) {
            return true;
        }
    }
    return filtersAdded;
}

AbstractImporter::TypeSupportedOptions IcedoveImportData::supportedOption()
{
    TypeSupportedOptions options;
    options |= AbstractImporter::Mails;
    options |= AbstractImporter::Filters;
    options |= AbstractImporter::Settings;
    options |= AbstractImporter::AddressBooks;
    return options;
}
