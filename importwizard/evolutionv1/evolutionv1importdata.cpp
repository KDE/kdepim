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

#include "evolutionv1/evolutionv1importdata.h"
#include "mailimporter/filter_evolution.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"

#include <KLocalizedString>

#include <QDir>


Evolutionv1ImportData::Evolutionv1ImportData(ImportWizard *parent)
    :AbstractImporter(parent)
{
    mPath = MailImporter::FilterEvolution::defaultSettingsPath();
}

Evolutionv1ImportData::~Evolutionv1ImportData()
{
}


bool Evolutionv1ImportData::foundMailer() const
{
    QDir directory( mPath );
    if ( directory.exists() )
        return true;
    return false;
}

QString Evolutionv1ImportData::name() const
{
    return QLatin1String("Evolution 1.x");
}

bool Evolutionv1ImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();
    MailImporter::FilterEvolution evolution;
    evolution.setFilterInfo( info );
    info->setStatusMessage(i18n("Import in progress"));
    const QString mailsPath = mPath;
    QDir directory(mailsPath);
    if (directory.exists())
        evolution.importMails(mailsPath);
    else
        evolution.import();
    info->setStatusMessage(i18n("Import finished"));

    delete info;
    return true;
}

AbstractImporter::TypeSupportedOptions Evolutionv1ImportData::supportedOption()
{
    TypeSupportedOptions options;
    options |=AbstractImporter::Mails;
    return options;
}
