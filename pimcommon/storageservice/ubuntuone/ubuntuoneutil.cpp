/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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


#include "ubuntuoneutil.h"

#include <qjson/parser.h>
#include <QVariant>
#include <QDebug>

QStringList PimCommon::UbuntuOneUtil::parseListFolder(const QString &data)
{
    QJson::Parser parser;
    bool ok;

    QMap<QString, QVariant> info = parser.parse(data.toUtf8(), &ok).toMap();
    qDebug()<<" info "<<info;
    QStringList listFolder;
    if (info.contains(QLatin1String("user_node_paths"))) {
        qDebug()<<" list folder "<<info.value(QLatin1String("user_node_paths"));
        QList<QVariant> lst = info.value(QLatin1String("user_node_paths")).toList();
        Q_FOREACH (const QVariant &v, lst)
            listFolder.append(v.toString());
    }
    return listFolder;
}
