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
#include <KTemporaryFile>
#include <KDebug>
#include <QDomDocument>

using namespace MessageComposer;

ImportLibreOfficeAutocorrection::ImportLibreOfficeAutocorrection(QWidget *parent)
    : ImportAbstractAutocorrection(parent), mArchive(0)
{
}

ImportLibreOfficeAutocorrection::~ImportLibreOfficeAutocorrection()
{
  closeArchive();
}

void ImportLibreOfficeAutocorrection::closeArchive()
{
  if (mArchive && mArchive->isOpen()) {
    mArchive->close();
  }
  delete mArchive;
}

bool ImportLibreOfficeAutocorrection::import(const QString& fileName)
{
    closeArchive();
    mArchive = new KZip(fileName);
    const bool result = mArchive->open(QIODevice::ReadOnly);
    if(result) {
      importAutoCorrectionFile();
      return true;
    } else {
      KMessageBox::error(mParent,i18n("Archive cannot be opened in read mode."),i18n("Import LibreOffice Autocorrection File"));
      return false;
    }
}

void ImportLibreOfficeAutocorrection::importAutoCorrectionFile()
{
  const KArchiveDirectory* archiveDirectory = mArchive->directory();
  //Replace word
  importFile(DOCUMENT, archiveDirectory);

  //No tread as end of line
  importFile(SENTENCE, archiveDirectory);

  //Two upper letter
  importFile(WORD, archiveDirectory);
}

bool ImportLibreOfficeAutocorrection::importFile(Type type, const KArchiveDirectory* archiveDirectory)
{
  const KArchiveEntry* documentList = 0;
  switch (type) {
  case DOCUMENT:
      documentList = archiveDirectory->entry(QLatin1String("DocumentList.xml"));
      break;
  case SENTENCE:
      documentList = archiveDirectory->entry(QLatin1String("SentenceExceptList.xml"));
      break;
  case WORD:
      documentList = archiveDirectory->entry(QLatin1String("WordExceptList.xml"));
      break;
  }

  if (documentList && documentList->isFile()) {
    const KArchiveFile* archiveFile = static_cast<const KArchiveFile*>(documentList);
    KTemporaryFile tmpFile;
    archiveFile->copyTo(tmpFile.fileName());
    QFile file(tmpFile.fileName());
    QDomDocument doc;
    if (loadDomElement( doc, &file )) {
      QDomElement list = doc.documentElement();
      if ( list.isNull() ) {
        kDebug() << "No list defined in "<<type;
      } else {
          for ( QDomElement e = list.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
            const QString tag = e.tagName();
            qDebug()<<" tag :"<<tag;
            if ( tag == QLatin1String( "block-list:block" ) ) {
            } else {
              kDebug() << " unknown tag " << tag;
            }
          }
      }
    }
  } else {
      return false;
  }
  return true;
}

bool ImportLibreOfficeAutocorrection::loadDomElement( QDomDocument &doc, QFile *file )
{
  QString errorMsg;
  int errorRow;
  int errorCol;
  if ( !doc.setContent( file, &errorMsg, &errorRow, &errorCol ) ) {
    kDebug() << "Unable to load document.Parse error in line " << errorRow
             << ", col " << errorCol << ": " << errorMsg;
    return false;
  }
  return true;
}
