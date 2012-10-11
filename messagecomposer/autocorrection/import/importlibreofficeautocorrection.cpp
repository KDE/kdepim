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
  if(result) {
    importAutoCorrectionFile();
  } else {
    KMessageBox::error(0,i18n("Archive cannot be opened in read mode."),i18n("Import LibreOffice Autocorrection File"));
  }
}

ImportLibreOfficeAutocorrection::~ImportLibreOfficeAutocorrection()
{
  if (mArchive && mArchive->isOpen()) {
    mArchive->close();
  }
  delete mArchive;
}

void ImportLibreOfficeAutocorrection::importAutoCorrectionFile()
{
  const KArchiveDirectory* mArchiveDirectory = mArchive->directory();
  //Replace word
  const KArchiveEntry* documentList = mArchiveDirectory->entry(QLatin1String("DocumentList.xml"));
  if (documentList && documentList->isFile()) {
    const KArchiveFile* file = static_cast<const KArchiveFile*>(documentList);

  }

  //No tread as end of line
  const KArchiveEntry* sentenceExceptList = mArchiveDirectory->entry(QLatin1String("SentenceExceptList.xml"));
  if (sentenceExceptList && sentenceExceptList->isFile()) {
    const KArchiveFile* file = static_cast<const KArchiveFile*>(sentenceExceptList);

  }

  //Two upper letter
  const KArchiveEntry* wordExceptList = mArchiveDirectory->entry(QLatin1String("WordExceptList.xml"));
  if (wordExceptList && wordExceptList->isFile()) {
    const KArchiveFile* file = static_cast<const KArchiveFile*>(wordExceptList);

  }
}
