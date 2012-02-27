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

#include "pimimportabstract.h"
#include "importmailpage.h"
#include "importfilterinfogui.h"

#include "mailimporter/filterinfo.h"

PimImportAbstract::PimImportAbstract(ImportMailPage *parent)
    :mMailPage(parent)
{
}

PimImportAbstract::~PimImportAbstract()
{
}


bool PimImportAbstract::importSettings()
{
  return false;
}

bool PimImportAbstract::importMails()
{
  return false;
}

bool PimImportAbstract::importFilters()
{
  return false;
}

bool PimImportAbstract::importAddressBook()
{
  return false;
}

MailImporter::FilterInfo* PimImportAbstract::initializeInfo()
{
    MailImporter::FilterInfo *info = new MailImporter::FilterInfo();
    ImportFilterInfoGui *infoGui = new ImportFilterInfoGui(mMailPage);
    info->setFilterInfoGui(infoGui);
    info->setRootCollection( mMailPage->selectedCollection() );
    return info;
}

  
