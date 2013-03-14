/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "grantleethememanager.h"

#include <KDirWatch>

#include <QMap>


using namespace MessageViewer;

class GrantleeThemeManager::Private
{
public:
    Private(const QString &path, GrantleeThemeManager *qq)
        : themePath(path),
          q(qq)
    {
        watch = new KDirWatch( q );
        q->connect( watch, SIGNAL(dirty(QString)), SLOT(directoryChanged()) );
    }

    void directoryChanged()
    {
        themes.clear();
        //TODO
    }

    QMap<QString, QString> themes;
    QString themePath;
    KDirWatch *watch;
    GrantleeThemeManager *q;
};

GrantleeThemeManager::GrantleeThemeManager(const QString &path, QObject *parent)
    : QObject(parent), d(new Private(path,this))
{
}

GrantleeThemeManager::~GrantleeThemeManager()
{
    delete d;
}


#include "grantleethememanager.moc"
