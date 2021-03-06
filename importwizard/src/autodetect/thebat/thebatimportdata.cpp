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

#include "thebat/thebatimportdata.h"
#include "mailimporter/filterthebat.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"
#include "importwizard.h"

#include <KLocalizedString>

#include <QDir>

TheBatImportData::TheBatImportData(ImportWizard *parent)
    : AbstractImporter(parent)
{
    //TODO fix it
    mPath = QDir::homePath();
}

TheBatImportData::~TheBatImportData()
{
}

bool TheBatImportData::foundMailer() const
{
    QDir directory(mPath);
    if (directory.exists()) {
        return true;
    }
    return false;
}

QString TheBatImportData::name() const
{
    return QStringLiteral("TheBat");
}

bool TheBatImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();

    MailImporter::FilterTheBat thebat;
    thebat.setFilterInfo(info);
    info->setStatusMessage(i18n("Import in progress"));
    QDir directory(mPath);
    if (directory.exists()) {
        thebat.importMails(mPath);
    } else {
        thebat.import();
    }
    info->setStatusMessage(i18n("Import finished"));

    delete info;
    return true;
}

AbstractImporter::TypeSupportedOptions TheBatImportData::supportedOption()
{
    TypeSupportedOptions options;
    options |= AbstractImporter::Mails;
    return options;
}
