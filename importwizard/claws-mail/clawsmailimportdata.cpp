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

#include "claws-mail/clawsmailimportdata.h"
#include "claws-mail/clawsmailsettings.h"
#include "claws-mail/clawsmailaddressbook.h"
#include "mailimporter/filter_clawsmail.h"
#include "mailcommon/filter/filterimporter/filterimporterclawsmail_p.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"
#include "importwizard.h"

#include <KLocale>

#include <QDir>


ClawsMailImportData::ClawsMailImportData(ImportWizard*parent)
    :AbstractImporter(parent)
{
    mPath = MailImporter::FilterClawsMail::defaultSettingsPath();
}

ClawsMailImportData::~ClawsMailImportData()
{
}


bool ClawsMailImportData::foundMailer() const
{
    QDir directory( mPath );
    if ( directory.exists() )
        return true;
    return false;
}

QString ClawsMailImportData::name() const
{
    return QLatin1String("Claws Mails");
}

bool ClawsMailImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();

    MailImporter::FilterClawsMail clawsMail;
    clawsMail.setFilterInfo( info );
    info->setStatusMessage(i18n("Import in progress"));
    const QString mailsPath = clawsMail.localMailDirPath();
    QDir directory(mailsPath);
    if (directory.exists())
        clawsMail.importMails(mailsPath);
    else
        clawsMail.import();
    info->setStatusMessage(i18n("Import finished"));


    delete info;
    return true;
}

bool ClawsMailImportData::importAddressBook()
{
    const QDir addressbookDir(mPath + QLatin1String("addrbook/"));
    ClawsMailAddressBook account( addressbookDir, mImportWizard );
    return true;
}

bool ClawsMailImportData::importSettings()
{
    const QString accountFile = mPath + QLatin1String("accountrc");
    if ( QFile( accountFile ).exists() ) {
        ClawsMailSettings account( mImportWizard );
        account.importSettings(accountFile, mPath);
    } else {
        addImportSettingsInfo(i18n("Claws Mail settings not found."));
    }
    return true;
}

bool ClawsMailImportData::importFilters()
{
    const QString filterPath(mPath + QLatin1String("matcherrc"));
    return addFilters( filterPath, MailCommon::FilterImporterExporter::ClawsMailFilter );
}


AbstractImporter::TypeSupportedOptions ClawsMailImportData::supportedOption()
{
    TypeSupportedOptions options;
    options |=AbstractImporter::Mails;
    options |=AbstractImporter::AddressBooks;
    options |=AbstractImporter::Settings;
    options |=AbstractImporter::Filters;
    return options;
}
