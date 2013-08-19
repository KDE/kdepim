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

#include "evolutionv2/evolutionv2importdata.h"
#include "mailimporter/filter_evolution_v2.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"

#include <KLocale>

#include <QDir>
#include <QWidget>


Evolutionv2ImportData::Evolutionv2ImportData(ImportWizard*parent)
    :AbstractImporter(parent)
{
    mPath = MailImporter::FilterEvolution_v2::defaultSettingsPath();
}

Evolutionv2ImportData::~Evolutionv2ImportData()
{
}


bool Evolutionv2ImportData::foundMailer() const
{
    QDir directory( mPath );
    if ( directory.exists() )
        return true;
    return false;
}

QString Evolutionv2ImportData::name() const
{
    return QLatin1String("Evolution 2.x");
}

bool Evolutionv2ImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();

    MailImporter::FilterEvolution_v2 evolution;
    evolution.setFilterInfo( info );
    info->setStatusMessage(i18n("Import in progress"));
    const QString mailsPath = mPath;
    QDir directory(mailsPath);
    if(directory.exists())
        evolution.importMails(mailsPath);
    else
        evolution.import();
    info->setStatusMessage(i18n("Import finished"));

    delete info;
    return true;
}

AbstractImporter::TypeSupportedOptions Evolutionv2ImportData::supportedOption()
{
    TypeSupportedOptions options;
    options |=AbstractImporter::Mails;
    return options;
}
