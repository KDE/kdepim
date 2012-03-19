/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "evolutionv3importdata.h"
#include "mailimporter/filter_evolution_v3.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"
#include "mailcommon/filter/filterimporterexporter.h"
#include "importwizard.h"

#include <KLocale>

#include <QDir>
#include <QWidget>


Evolutionv3ImportData::Evolutionv3ImportData(ImportWizard*parent)
    :AbstractImporter(parent)
{
    mPath = MailImporter::FilterEvolution_v3::defaultPath();
}

Evolutionv3ImportData::~Evolutionv3ImportData()
{
}


bool Evolutionv3ImportData::foundMailer() const
{
  QDir directory( mPath );
  if ( directory.exists() )
    return true;
  return false;
}

QString Evolutionv3ImportData::name() const
{
  return QLatin1String("Evolution 3.x");
}

bool Evolutionv3ImportData::importSettings()
{
  return false;
}

bool Evolutionv3ImportData::importMails()
{
    MailImporter::FilterInfo *info = initializeInfo();
    info->clear(); // Clear info from last time


    MailImporter::FilterEvolution_v3 evolution;
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

bool Evolutionv3ImportData::importFilters()
{
  const QString filterPath = QDir::homePath() +QLatin1String("/.config/evolution/mail/filter.xml");
  return addFilters( filterPath, MailCommon::FilterImporterExporter::EvolutionFilter );
}

bool Evolutionv3ImportData::importAddressBook()
{
  return false;
}

AbstractImporter::TypeSupportedOptions Evolutionv3ImportData::supportedOption()
{
  TypeSupportedOptions options;
  options |=AbstractImporter::Mails;
  options |=AbstractImporter::Filters;
  return options;
}
