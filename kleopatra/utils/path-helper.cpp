/* -*- mode: c++; c-basic-offset:4 -*-
    utils/path-helper.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#include "path-helper.h"

#include <kleo/stl_util.h>

#include <kleo/exception.h>

#include <QDebug>
#include <KLocalizedString>

#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QDir>

#include <boost/bind.hpp>

#include <algorithm>

using namespace Kleo;
using namespace boost;

static QString commonPrefix(const QString &s1, const QString &s2)
{
    return QString(s1.data(), std::mismatch(s1.data(), s1.data() + std::min(s1.size(), s2.size()), s2.data()).first - s1.data());
}

static QString longestCommonPrefix(const QStringList &sl)
{
    if (sl.empty()) {
        return QString();
    }
    QString result = sl.front();
    Q_FOREACH (const QString &s, sl) {
        result = commonPrefix(s, result);
    }
    return result;
}

QString Kleo::heuristicBaseDirectory(const QStringList &fileNames)
{
    QStringList dirs;
    Q_FOREACH (const QString &fileName, fileNames) {
        dirs.push_back(QFileInfo(fileName).path() + QLatin1Char('/'));
    }
    qDebug() << "dirs" << dirs;
    const QString candidate = longestCommonPrefix(dirs);
    const int idx = candidate.lastIndexOf(QLatin1Char('/'));
    return candidate.left(idx);
}

QStringList Kleo::makeRelativeTo(const QString &base, const QStringList &fileNames)
{

    if (base.isEmpty()) {
        return fileNames;
    } else {
        return makeRelativeTo(QDir(base), fileNames);
    }

}

QStringList Kleo::makeRelativeTo(const QDir &baseDir, const QStringList &fileNames)
{
    return kdtools::transform<QStringList>
           (fileNames,
            boost::bind(&QDir::relativeFilePath, &baseDir, _1));
}

void Kleo::recursivelyRemovePath(const QString &path)
{
    const QFileInfo fi(path);
    if (fi.isDir()) {
        QDir dir(path);
        Q_FOREACH (const QString &fname, dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
            recursivelyRemovePath(dir.filePath(fname));
        }
        const QString dirName = fi.fileName();
        dir.cdUp();
        if (!dir.rmdir(dirName)) {
            throw Exception(GPG_ERR_EPERM, i18n("Cannot remove directory %1", path));
        }
    } else {
        QFile file(path);
        if (!file.remove()) {
            throw Exception(GPG_ERR_EPERM, i18n("Cannot remove file %1: %2", path, file.errorString()));
        }
    }
}
