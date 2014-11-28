/* -*- mode: c++; c-basic-offset:4 -*-
    utils/filedialog.cpp

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

#include "filedialog.h"

#ifdef QT_NO_FILEDIALOG
#include <KFileDialog>
#include <KUrl>
#include <QTextStream>
#endif
#include <QFileDialog>
#include <QDir>
#include <QMap>

using namespace Kleo;

namespace
{
typedef QMap<QString, QString> Map;
Q_GLOBAL_STATIC(Map, dir_id_2_dir_map)
}

static QString dir(const QString &id)
{
    const QString dir = (*dir_id_2_dir_map())[id];
    if (dir.isEmpty()) {
        return QDir::homePath();
    } else {
        return dir;
    }
}

static void update(const QString &fname, const QString &id)
{
    if (!fname.isEmpty()) {
        (*dir_id_2_dir_map())[ id ] = QFileInfo(fname).absolutePath();
    }
}

#ifdef QT_NO_FILEDIALOG
static QString qt2KdeFilter(const QString &f)
{
    QString               filter;
    QTextStream           str(&filter, QIODevice::WriteOnly);
    QStringList           list(f.split(";;").replaceInStrings("/", "\\/"));
    QStringList::const_iterator it(list.begin()),
                end(list.end());
    bool                  first = true;

    for (; it != end; ++it) {
        int ob = (*it).lastIndexOf('('),
            cb = (*it).lastIndexOf(')');

        if (-1 != cb && ob < cb) {
            if (first) {
                first = false;
            } else {
                str << '\n';
            }
            str << (*it).mid(ob + 1, (cb - ob) - 1) << '|' << (*it).mid(0, ob);
        }
    }

    return filter;
}
#endif

QString FileDialog::getExistingDirectory(QWidget *parent, const QString &caption, const QString &dirID)
{
#ifndef QT_NO_FILEDIALOG
    const QString fname = QFileDialog::getExistingDirectory(parent, caption, dir(dirID));
#else
    const QString fname = QFileDialog::getExistingDirectory(parent, caption ,  dir(dirID));
#endif
    update(fname, dirID);
    return fname;
}

QString FileDialog::getOpenFileName(QWidget *parent, const QString &caption, const QString &dirID, const QString &filter)
{
#ifndef QT_NO_FILEDIALOG
    const QString fname = QFileDialog::getOpenFileName(parent, caption, dir(dirID), filter);
#else
    const QString fname = QFileDialog::getOpenFileName(parent, caption ,  dir(dirID), qt2KdeFilter(filter));
#endif
    update(fname, dirID);
    return fname;
}

QStringList FileDialog::getOpenFileNames(QWidget *parent, const QString &caption, const QString &dirID, const QString &filter)
{
#ifndef QT_NO_FILEDIALOG
    const QStringList files = QFileDialog::getOpenFileNames(parent, caption, dir(dirID), filter);
#else
    const QStringList files = KFileDialog::getOpenFileNames(dir(dirID), qt2KdeFilter(filter), parent, caption);
#endif
    if (!files.empty()) {
        update(files.front(), dirID);
    }
    return files;
}

QString FileDialog::getSaveFileName(QWidget *parent, const QString &caption, const QString &dirID, const QString &filter)
{
#ifndef QT_NO_FILEDIALOG
    const QString fname = QFileDialog::getSaveFileName(parent, caption, dir(dirID), filter);
#else
    const QString fname = KFileDialog::getSaveFileName(dirID.isEmpty() ? QUrl() : dir(dirID), qt2KdeFilter(filter), parent, caption);
#endif
    update(fname, dirID);
    return fname;
}

QString FileDialog::getSaveFileNameEx(QWidget *parent, const QString &caption, const QString &dirID, const QString &proposedFileName, const QString &filter)
{
    if (proposedFileName.isEmpty())
#ifndef QT_NO_FILEDIALOG
        return getSaveFileName(parent, caption, dirID, filter);
    const QString fname = QFileDialog::getSaveFileName(parent, caption, QDir(dir(dirID)).filePath(proposedFileName), filter);
#else
        return getSaveFileName(parent, caption, QString(), filter);
    const QString fname = KFileDialog::getSaveFileName(QDir(dir(dirID)).filePath(proposedFileName), qt2KdeFilter(filter), parent, caption);
#endif
    update(fname, dirID);
    return fname;
}
