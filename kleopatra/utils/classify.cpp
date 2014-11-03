/* -*- mode: c++; c-basic-offset:4 -*-
    utils/classify.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "classify.h"

#include "fileoperationspreferences.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QtAlgorithms>
#include <QByteArrayMatcher>

#include <boost/range.hpp>

#ifdef __GLIBCXX__
# include <ext/algorithm>
#endif

#include <functional>

using namespace boost;
using namespace Kleo::Class;

namespace
{

const unsigned int ExamineContentHint = 0x8000;

static const struct _classification {
    char extension[4];
    unsigned int classification;
} classifications[] = {
    // ordered by extension
    { "arl", CMS    | Binary  | CertificateRevocationList },
    { "asc", OpenPGP |  Ascii  | OpaqueSignature | DetachedSignature | CipherText | AnyCertStoreType | ExamineContentHint },
    { "crl", CMS    | Binary  | CertificateRevocationList },
    { "crt", CMS    | Binary  | Certificate },
    { "der", CMS    | Binary  | Certificate | CertificateRevocationList },
    { "gpg", OpenPGP | Binary  | OpaqueSignature | CipherText | AnyCertStoreType },
    { "p10", CMS    |  Ascii  | CertificateRequest },
    { "p12", CMS    | Binary  | ExportedPSM },
    { "p7c", CMS    | Binary  | Certificate  },
    { "p7m", CMS    | Binary  | CipherText },
    { "p7s", CMS    | Binary  | AnySignature },
    { "pem", CMS    |  Ascii  | AnyType | ExamineContentHint },
    { "pgp", OpenPGP | Binary  | OpaqueSignature | CipherText | AnyCertStoreType },
    { "sig", OpenPGP | AnyFormat | DetachedSignature },
};

static const unsigned int defaultClassification = NoClass;

template <template <typename U> class Op>
struct ByExtension {
    typedef bool result_type;

    template <typename T>
    bool operator()(const T &lhs, const T &rhs) const
    {
        return Op<int>()(qstricmp(lhs.extension, rhs.extension), 0);
    }
    template <typename T>
    bool operator()(const T &lhs, const char *rhs) const
    {
        return Op<int>()(qstricmp(lhs.extension, rhs), 0);
    }
    template <typename T>
    bool operator()(const char *lhs, const T &rhs) const
    {
        return Op<int>()(qstricmp(lhs, rhs.extension), 0);
    }
    bool operator()(const char *lhs, const char *rhs) const
    {
        return Op<int>()(qstricmp(lhs, rhs), 0);
    }
};

static const struct _content_classification {
    char content[28];
    unsigned int classification;
} content_classifications[] = {
    { "CERTIFICATE",       Certificate },
    { "ENCRYPTED MESSAGE", CipherText  },
    { "MESSAGE",           OpaqueSignature | CipherText },
    { "PKCS12",            ExportedPSM },
    { "PRIVATE KEY BLOCK", ExportedPSM },
    { "PUBLIC KEY BLOCK",  Certificate },
    { "SIGNATURE",         DetachedSignature },
    { "SIGNED MESSAGE",    ClearsignedMessage | DetachedSignature },
};

template <template <typename U> class Op>
struct ByContent {
    typedef bool result_type;

    const unsigned int N;
    explicit ByContent(unsigned int n) : N(n) {}

    template <typename T>
    bool operator()(const T &lhs, const T &rhs) const
    {
        return Op<int>()(qstrncmp(lhs.content, rhs.content, N), 0);
    }
    template <typename T>
    bool operator()(const T &lhs, const char *rhs) const
    {
        return Op<int>()(qstrncmp(lhs.content, rhs, N), 0);
    }
    template <typename T>
    bool operator()(const char *lhs, const T &rhs) const
    {
        return Op<int>()(qstrncmp(lhs, rhs.content, N), 0);
    }
    bool operator()(const char *lhs, const char *rhs) const
    {
        return Op<int>()(qstrncmp(lhs, rhs, N), 0);
    }
};

}

unsigned int Kleo::classify(const QStringList &fileNames)
{
    if (fileNames.empty()) {
        return 0;
    }
    unsigned int result = classify(fileNames.front());
    Q_FOREACH (const QString &fileName, fileNames) {
        result &= classify(fileName);
    }
    return result;
}

unsigned int Kleo::classify(const QString &filename)
{
#ifdef __GLIBCXX__
    assert(__gnu_cxx::is_sorted(begin(classifications), end(classifications), ByExtension<std::less>()));
#endif

    const QFileInfo fi(filename);

    const _classification *const it = qBinaryFind(begin(classifications), end(classifications),
                                      fi.suffix().toLatin1().constData(),
                                      ByExtension<std::less>());
    if (it != end(classifications))
        if (!(it->classification & ExamineContentHint)) {
            return it->classification;
        }

    const unsigned int bestGuess =
        it == end(classifications) ? defaultClassification
        /* else */                   : it->classification ;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return bestGuess;
    }

    const unsigned int contentClassification = classifyContent(file.read(4096));
    if (contentClassification != defaultClassification) {
        return contentClassification;
    } else {
        return bestGuess;
    }
}

unsigned int Kleo::classifyContent(const QByteArray &data)
{
#ifdef __GLIBCXX__
    assert(__gnu_cxx::is_sorted(begin(content_classifications), end(content_classifications), ByContent<std::less>(100)));
#endif

    static const char beginString[] = "-----BEGIN ";
    static const QByteArrayMatcher beginMatcher(beginString);
    int pos = beginMatcher.indexIn(data);
    if (pos < 0) {
        return defaultClassification;
    }
    pos += sizeof beginString - 1;

    const bool pgp = qstrncmp(data.data() + pos, "PGP ", 4) == 0;
    if (pgp) {
        pos += 4;
    }

    const int epos = data.indexOf("-----\n", pos);
    if (epos < 0) {
        return defaultClassification;
    }

    const _content_classification *const cit
        = qBinaryFind(begin(content_classifications), end(content_classifications),
                      data.data() + pos, ByContent<std::less>(epos - pos));

    if (cit == end(content_classifications)) {
        return defaultClassification;
    } else {
        return cit->classification | (pgp ? OpenPGP : CMS);
    }
}

QString Kleo::printableClassification(unsigned int classification)
{
    QStringList parts;
    if (classification & CMS) {
        parts.push_back(QLatin1String("CMS"));
    }
    if (classification & OpenPGP) {
        parts.push_back(QLatin1String("OpenPGP"));
    }
    if (classification & Binary) {
        parts.push_back(QLatin1String("Binary"));
    }
    if (classification & Ascii) {
        parts.push_back(QLatin1String("Ascii"));
    }
    if (classification & DetachedSignature) {
        parts.push_back(QLatin1String("DetachedSignature"));
    }
    if (classification & OpaqueSignature) {
        parts.push_back(QLatin1String("OpaqueSignature"));
    }
    if (classification & ClearsignedMessage) {
        parts.push_back(QLatin1String("ClearsignedMessage"));
    }
    if (classification & CipherText) {
        parts.push_back(QLatin1String("CipherText"));
    }
    if (classification & Certificate) {
        parts.push_back(QLatin1String("Certificate"));
    }
    if (classification & ExportedPSM) {
        parts.push_back(QLatin1String("ExportedPSM"));
    }
    if (classification & CertificateRequest) {
        parts.push_back(QLatin1String("CertificateRequest"));
    }
    return parts.join(QLatin1String(", "));
}

static QString chopped(QString s, unsigned int n)
{
    s.chop(n);
    return s;
}

/*!
  \return the data file that corresponds to the signature file \a
  signatureFileName, or QString(), if no such file can be found.
*/
QString Kleo::findSignedData(const QString &signatureFileName)
{
    if (!mayBeDetachedSignature(signatureFileName)) {
        return QString();
    }
    const QString baseName = chopped(signatureFileName, 4);
    return QFile::exists(baseName) ? baseName : QString() ;
}

/*!
  \return all (existing) candiate signature files for \a signedDataFileName

  Note that there can very well be more than one such file, e.g. if
  the same data file was signed by both CMS and OpenPGP certificates.
*/
QStringList Kleo::findSignatures(const QString &signedDataFileName)
{
    QStringList result;
    for (unsigned int i = 0, end = size(classifications) ; i < end ; ++i)
        if (classifications[i].classification & DetachedSignature) {
            const QString candiate = signedDataFileName + QLatin1Char('.') + QLatin1String(classifications[i].extension);
            if (QFile::exists(candiate)) {
                result.push_back(candiate);
            }
        }
    return result;
}

/*!
  \return the (likely) output filename for \a inputFileName, or
  "inputFileName.out" if none can be determined.
*/
QString Kleo::outputFileName(const QString &inputFileName)
{
    const QFileInfo fi(inputFileName);

    if (qBinaryFind(begin(classifications), end(classifications),
                    fi.suffix().toLatin1().constData(),
                    ByExtension<std::less>()) == end(classifications)) {
        return inputFileName + QLatin1String(".out");
    } else {
        return chopped(inputFileName, 4);
    }
}

/*!
  \return the commonly used extension for files of type
  \a classification, or NULL if none such exists.
*/
const char *Kleo::outputFileExtension(unsigned int classification)
{

    if (classification & OpenPGP) {
        FileOperationsPreferences filePrefs;
        if (filePrefs.usePGPFileExt()) {
            return "pgp";
        }
    }

    for (unsigned int i = 0 ; i < sizeof classifications / sizeof * classifications ; ++i)
        if ((classifications[i].classification & classification) == classification) {
            return classifications[i].extension;
        }
    return 0;
}
