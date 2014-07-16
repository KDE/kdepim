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

#include "sylpheed/sylpheedimportdata.h"
#include "mailimporter/filter_sylpheed.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"
#include "mailcommon/filter/filterimporterexporter.h"
#include "importwizard.h"
#include "sylpheed/sylpheedsettings.h"
#include "sylpheed/sylpheedaddressbook.h"

#include <KLocale>

#include <QDir>


SylpheedImportData::SylpheedImportData(ImportWizard*parent)
    :AbstractImporter(parent)
{
    mPath = MailImporter::FilterSylpheed::defaultSettingsPath();
}

SylpheedImportData::~SylpheedImportData()
{
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
    const QString accountFile = mPath + QLatin1String("/accountrc");
    if ( QFile( accountFile ).exists() ) {
        SylpheedSettings account( mImportWizard );
        account.importSettings(accountFile, mPath);
    } else {
        addImportSettingsInfo(i18n("Sylpheed settings not found."));
    }
    return true;
}

bool SylpheedImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();

    MailImporter::FilterSylpheed sylpheed;
    sylpheed.setFilterInfo( info );
    info->setStatusMessage(i18n("Import in progress"));
    const QString mailsPath = sylpheed.localMailDirPath();
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
    const QString filterPath = mPath + QLatin1String("/filter.xml");
    return addFilters( filterPath, MailCommon::FilterImporterExporter::SylpheedFilter );
}

bool SylpheedImportData::importAddressBook()
{
    const QDir addressbookDir(mPath);
    SylpheedAddressBook account( addressbookDir, mImportWizard );
    return true;
}

AbstractImporter::TypeSupportedOptions SylpheedImportData::supportedOption()
{
    TypeSupportedOptions options;
    options |=AbstractImporter::Mails;
    options |=AbstractImporter::Filters;
    options |=AbstractImporter::Settings;
    options |=AbstractImporter::AddressBooks;
    return options;
}
