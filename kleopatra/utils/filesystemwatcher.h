/* -*- mode: c++; c-basic-offset:4 -*-
    filesystemwatcher.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_UTILS_FILESYSTEMWATCHER_H__
#define __KLEOPATRA_UTILS_FILESYSTEMWATCHER_H__

#include <QObject>

#include <utils/pimpl_ptr.h>

class QString;
class QStringList;

namespace Kleo
{

class FileSystemWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileSystemWatcher(QObject *parent = 0);
    explicit FileSystemWatcher(const QStringList &paths, QObject *parent = 0);
    ~FileSystemWatcher();

    void setDelay(int ms);
    int delay() const;

    void setEnabled(bool enable);
    bool isEnabled() const;

    void addPaths(const QStringList &paths);
    void addPath(const QString &path);

    void blacklistFiles(const QStringList &patterns);
    void whitelistFiles(const QStringList &patterns);

    QStringList directories() const;
    QStringList files() const;
    void removePaths(const QStringList &path);
    void removePath(const QString &path);

Q_SIGNALS:
    void directoryChanged(const QString &path);
    void fileChanged(const QString &path);
    void triggered();

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void onFileChanged(QString))
    Q_PRIVATE_SLOT(d, void onDirectoryChanged(QString))
    Q_PRIVATE_SLOT(d, void onTimeout())
};
}

#endif // __KLEOPATRA_UTILS_FILESYSTEMWATCHER_H__
