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

#include "importlibreofficeautocorrection.h"
#include <QFile>
#include <KZip>
#include <KLocalizedString>
#include <KMessageBox>
#include <QTemporaryDir>
#include <QDebug>
#include <QDomDocument>
#include <QDir>

using namespace PimCommon;

ImportLibreOfficeAutocorrection::ImportLibreOfficeAutocorrection(QWidget *parent)
    : ImportAbstractAutocorrection(parent), mArchive(0), mTempDir(0)
{
}

ImportLibreOfficeAutocorrection::~ImportLibreOfficeAutocorrection()
{
    closeArchive();
}

void ImportLibreOfficeAutocorrection::closeArchive()
{
    if (mArchive) {
        if (mArchive->isOpen()) {
            mArchive->close();
        }
        delete mArchive;
        mArchive = 0;
    }

    delete mTempDir;
    mTempDir = 0;
}

bool ImportLibreOfficeAutocorrection::import(const QString &fileName, LoadAttribute loadAttribute)
{
    //We Don't have it in LibreOffice
    if (loadAttribute == SuperScript) {
        return false;
    }
    closeArchive();
    mArchive = new KZip(fileName);
    const bool result = mArchive->open(QIODevice::ReadOnly);
    if (result) {
        importAutoCorrectionFile();
        return true;
    } else {
        KMessageBox::error(mParent, i18n("Archive cannot be opened in read mode."), i18n("Import LibreOffice Autocorrection File"));
        return false;
    }
}

void ImportLibreOfficeAutocorrection::importAutoCorrectionFile()
{
    mTempDir = new QTemporaryDir();
    const KArchiveDirectory *archiveDirectory = mArchive->directory();
    //Replace word
    importFile(DOCUMENT, archiveDirectory);

    //No tread as end of line
    importFile(SENTENCE, archiveDirectory);

    //Two upper letter
    importFile(WORD, archiveDirectory);
}

bool ImportLibreOfficeAutocorrection::importFile(Type type, const KArchiveDirectory *archiveDirectory)
{
    const KArchiveEntry *documentList = 0;

    QString archiveFileName;
    switch (type) {
    case DOCUMENT:
        archiveFileName = QLatin1String("DocumentList.xml");
        break;
    case SENTENCE:
        archiveFileName = QLatin1String("SentenceExceptList.xml");
        break;
    case WORD:
        archiveFileName = QLatin1String("WordExceptList.xml");
        break;
    default:
        return false;
    }
    documentList = archiveDirectory->entry(archiveFileName);
    if (documentList && documentList->isFile()) {
        const KArchiveFile *archiveFile = static_cast<const KArchiveFile *>(documentList);
        archiveFile->copyTo(mTempDir->path());
        QFile file(mTempDir->path() + QDir::separator() + archiveFileName);
        QDomDocument doc;
        if (loadDomElement(doc, &file)) {
            QDomElement list = doc.documentElement();
            if (list.isNull()) {
                qDebug() << "No list defined in " << type;
            } else {
                for (QDomElement e = list.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
                    const QString tag = e.tagName();
                    if (tag == QLatin1String("block-list:block")) {
                        switch (type) {
                        case DOCUMENT:
                            if (e.hasAttribute(QLatin1String("block-list:abbreviated-name")) && e.hasAttribute(QLatin1String("block-list:name"))) {
                                mAutocorrectEntries.insert(e.attribute(QLatin1String("block-list:abbreviated-name")), e.attribute(QLatin1String("block-list:name")));
                            }
                            break;
                        case SENTENCE:
                            if (e.hasAttribute(QLatin1String("block-list:abbreviated-name"))) {
                                mTwoUpperLetterExceptions.insert(e.attribute(QLatin1String("block-list:abbreviated-name")));
                            }

                            break;
                        case WORD:
                            if (e.hasAttribute(QLatin1String("block-list:abbreviated-name"))) {
                                mUpperCaseExceptions.insert(e.attribute(QLatin1String("block-list:abbreviated-name")));
                            }
                            break;

                        }
                    } else {
                        qDebug() << " unknown tag " << tag;
                    }
                }
            }
        }
    } else {
        return false;
    }
    return true;
}

bool ImportLibreOfficeAutocorrection::loadDomElement(QDomDocument &doc, QFile *file)
{
    QString errorMsg;
    int errorRow;
    int errorCol;
    if (!doc.setContent(file, &errorMsg, &errorRow, &errorCol)) {
        qDebug() << "Unable to load document.Parse error in line " << errorRow
                 << ", col " << errorCol << ": " << errorMsg;
        return false;
    }
    return true;
}
