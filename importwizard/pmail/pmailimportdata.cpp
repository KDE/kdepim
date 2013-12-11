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

#include "pmail/pmailimportdata.h"
#include "pmail/pmailsettings.h"
#include "mailimporter/filter_pmail.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"
#include "importwizard.h"

#include <KLocale>

#include <QDir>


PMailImportData::PMailImportData(ImportWizard*parent)
    :AbstractImporter(parent)
{
    mPath = QDir::homePath();
}

PMailImportData::~PMailImportData()
{
}


bool PMailImportData::foundMailer() const
{
#ifdef Q_OS_WIN
    //TODO find a method to search it. Perhaps look at binary.
    QDir directory( mPath );
    if ( directory.exists() )
        return true;
#endif
    return false;
}

QString PMailImportData::name() const
{
    return QLatin1String("Pegasus Mail");
}

bool PMailImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();
    MailImporter::FilterPMail pmail;
    pmail.setFilterInfo( info );
    info->setStatusMessage(i18n("Import in progress"));
    QDir directory(mPath);
    if(directory.exists())
        pmail.importMails(mPath);
    else
        pmail.import();
    info->setStatusMessage(i18n("Import finished"));

    delete info;
    return true;
}

bool PMailImportData::importSettings()
{
    //TODO verify path
    const QString settingFile(mPath+QLatin1String("pmail.ini"));
    PMailSettings settings(settingFile,mImportWizard);
    return true;
}


AbstractImporter::TypeSupportedOptions PMailImportData::supportedOption()
{
    TypeSupportedOptions options;
    options |=AbstractImporter::Mails;
    //options |=AbstractImporter::Settings;
    return options;
}
