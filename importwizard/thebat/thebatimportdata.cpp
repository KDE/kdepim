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

#include "thebat/thebatimportdata.h"
#include "mailimporter/filter_thebat.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"
#include "importwizard.h"

#include <KLocale>

#include <QDir>


TheBatImportData::TheBatImportData(ImportWizard*parent)
    :AbstractImporter(parent)
{
    //TODO fix it
    mPath = QDir::homePath();
}

TheBatImportData::~TheBatImportData()
{
}


bool TheBatImportData::foundMailer() const
{
#ifdef Q_OS_WIN
    QDir directory( mPath );
    if ( directory.exists() )
        return true;
#endif
    return false;
}

QString TheBatImportData::name() const
{
    return QLatin1String("TheBat");
}

bool TheBatImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();

    MailImporter::FilterTheBat thebat;
    thebat.setFilterInfo( info );
    info->setStatusMessage(i18n("Import in progress"));
    QDir directory(mPath);
    if (directory.exists())
        thebat.importMails(mPath);
    else
        thebat.import();
    info->setStatusMessage(i18n("Import finished"));

    delete info;
    return true;
}

AbstractImporter::TypeSupportedOptions TheBatImportData::supportedOption()
{
    TypeSupportedOptions options;
    options |=AbstractImporter::Mails;
    return options;
}
