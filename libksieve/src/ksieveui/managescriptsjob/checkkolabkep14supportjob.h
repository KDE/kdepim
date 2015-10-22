/*
 * Copyright (c) 2015 Sandro Knauß <knauss@kolabsys.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CHECKKOLABKEP14SUPPORTJOB_H
#define CHECKKOLABKEP14SUPPORTJOB_H

#include <QObject>
#include <QStringList>

#include "ksieveui_export.h"

#include <QUrl>

namespace KManageSieve
{
class SieveJob;
}

namespace KSieveUi
{
class CheckKolabKep14SupportJobPrivate;

/**
\brief Checks for support of Non-conflicting edits of Sieve scripts by multiple editors (KEP:14)

\par Introduction

This Kolab Enhancement Proposal defines the conventions and application behaviour to enable
non-conflicting edits of RFC 5228 Sieve scripts on the server by multiple editors.

The result is the ability to have different layers of Sieve management per user, with different
levels of required authorization, and the ability to provide users with multiple active scripts
which can be activated and deactivated as required, thus enabling splitting of the user-facing
Sieve functionality into more conveniently managed files.

Every sieve server can support KEP:14, because it is only a standardisation of filenames and a
way to support and enable mutliple scripts.

For more information on Kolab KEP:14 see:
 * http://wiki.kolab.org/KEP:14
 * http://git.kolabsys.com/keps/tree/KEP-0014.txt

*/

class KSIEVEUI_EXPORT CheckKolabKep14SupportJob : public QObject
{
    Q_OBJECT
public:
    explicit CheckKolabKep14SupportJob(QObject *parent = Q_NULLPTR);
    ~CheckKolabKep14SupportJob();

    void start();

    void setServerUrl(const QUrl &url);
    void setServerName(const QString &name);
    QString serverName() const;

    QStringList availableScripts() const;
    bool hasKep14Support() const;
    QUrl serverUrl() const;

Q_SIGNALS:
    void result(CheckKolabKep14SupportJob *, bool);

private:
    CheckKolabKep14SupportJobPrivate *d;

private Q_SLOTS:
    void slotCheckKep14Support(KManageSieve::SieveJob *job, bool success, const QStringList &availableScripts, const QString &activeScript);
};
}

#endif // CHECKKOLABKEP14SUPPORTJOB_H
