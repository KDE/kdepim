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

#include "thunderbirdimportdata.h"

#include <QDir>

ThunderbirdImportData::ThunderbirdImportData()
{
    mPath = QDir::homePath() + QLatin1String( "/.thunderbird/" );
}

ThunderbirdImportData::~ThunderbirdImportData()
{
}

bool ThunderbirdImportData::foundMailer() const
{
  QDir directory( mPath );
  if ( directory.exists() )
    return true;
  return false;
}

QString ThunderbirdImportData::name() const
{
  return QLatin1String("Thunderbird");
}

bool ThunderbirdImportData::importSettings()
{
  return false;
}

bool ThunderbirdImportData::importMails()
{
  return false;
}

bool ThunderbirdImportData::importFilters()
{
  return false;
}

bool ThunderbirdImportData::importAddressBook()
{
  return false;
}

PimImportAbstract::TypeSupportedOptions ThunderbirdImportData::supportedOption()
{
  TypeSupportedOptions options;
  options |=PimImportAbstract::Mails;
  options |=PimImportAbstract::Filters;
  return options;
}
