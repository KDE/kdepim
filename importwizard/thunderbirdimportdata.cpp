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


ThunderbirdImportData::ThunderbirdImportData()
{
}

ThunderbirdImportData::~ThunderbirdImportData()
{
}

bool ThunderbirdImportData::foundMailer() const
{
  return false;
}

void ThunderbirdImportData::importSettings()
{
}

void ThunderbirdImportData::importMails()
{
}

void ThunderbirdImportData::importFilters()
{
}

void ThunderbirdImportData::importAddressBook()
{
}

PimImportAbstract::TypeSupportedOptions ThunderbirdImportData::supportedOption()
{
  TypeSupportedOptions options;
  options |=PimImportAbstract::Mails;
  options |=PimImportAbstract::Filters;
  return options;
}
