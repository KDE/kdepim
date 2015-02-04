/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "baloocompletionemail.h"
#include <QMap>
#include <QDebug>
using namespace KPIM;

BalooCompletionEmail::BalooCompletionEmail()
{

}

void BalooCompletionEmail::setEmailList(const QStringList &lst)
{
    mListEmail = lst;
}

void BalooCompletionEmail::setExcludeDomain(const QStringList &lst)
{
    mExcludeDomain = lst;
}

void BalooCompletionEmail::setBlackList(const QStringList &lst)
{
    mBlackList = lst;
}

QStringList BalooCompletionEmail::cleanupEmailList() const
{
    if (mListEmail.isEmpty())
        return mListEmail;
    QMap<QString, QString> hashEmail;
    Q_FOREACH (const QString &email, mListEmail) {
        if (!mBlackList.contains(email)) {
            bool excludeMail = false;
            Q_FOREACH(const QString &excludeDomain, mExcludeDomain) {
                if (!excludeDomain.isEmpty()) {
                    if (email.endsWith(excludeDomain)) {
                        excludeMail = true;
                        continue;
                    }
                }
            }
            if (!excludeMail && !hashEmail.contains(email.toLower())) {
                hashEmail.insert(email.toLower(), email);
            }
        }
    }
    return hashEmail.values();
}
