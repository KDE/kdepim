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

#include "dropboxutil.h"
#include <QVariant>
#include <QLocale>
#include <QDateTime>
#include <QJsonParseError>

QStringList PimCommon::DropBoxUtil::getListFolder(const QString &data)
{
    QStringList listFolder;
    QJsonParseError error;
    const QJsonDocument json = QJsonDocument::fromJson(data.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || json.isNull()) {
        return listFolder;
    }
    const QMap<QString, QVariant> info = json.toVariant().toMap();

    if (info.contains(QLatin1String("contents"))) {
        const QVariantList lst = info.value(QLatin1String("contents")).toList();
        Q_FOREACH (const QVariant &variant, lst) {
            const QVariantMap qwer = variant.toMap();
            if (qwer.contains(QLatin1String("is_dir"))) {
                const bool value = qwer.value(QLatin1String("is_dir")).toBool();
                if (value) {
                    const QString name = qwer.value(QLatin1String("path")).toString();
                    listFolder.append(name);
                }
            }
        }
    }
    return listFolder;
}

QDateTime PimCommon::DropBoxUtil::convertToDateTime(QString dateTime)
{
    dateTime.chop(6);     // chop() removes the time zone
    QLocale locale(QLocale::C);
    const QDateTime t = QDateTime(locale.toDateTime(dateTime, QLatin1String("ddd, dd MMM yyyy hh:mm:ss")));
    return t;
}
