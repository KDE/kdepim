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

#include "importlibreofficeautocorrection.h"
#include <QFile>
#include <KZip>
#include <KLocale>
#include <KMessageBox>

using namespace MessageComposer;

ImportLibreOfficeAutocorrection::ImportLibreOfficeAutocorrection(const QString& fileName)
    : mArchive(new KZip(fileName))
{
  const bool result = mArchive->open(QIODevice::ReadOnly);
  if(!result) {
    KMessageBox::error(0,i18n("Archive cannot be opened in read mode."),i18n("Import LibreOffice Autocorrection File"));
  }
  importAutoCorrectionFile();
}

ImportLibreOfficeAutocorrection::~ImportLibreOfficeAutocorrection()
{
  if(mArchive && mArchive->isOpen()) {
    mArchive->close();
  }
  delete mArchive;
}

void ImportLibreOfficeAutocorrection::importAutoCorrectionFile()
{

}
