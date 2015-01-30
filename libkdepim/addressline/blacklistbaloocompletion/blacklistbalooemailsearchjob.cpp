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

#include "blacklistbalooemailsearchjob.h"

#include <baloo/pim/contactcompleter.h>
#include <QStringList>
using namespace KPIM;

BlackListBalooEmailSearchJob::BlackListBalooEmailSearchJob(QObject *parent)
    : QObject(parent)
{

}

BlackListBalooEmailSearchJob::~BlackListBalooEmailSearchJob()
{

}

bool BlackListBalooEmailSearchJob::start()
{
    const QString trimmedString = mSearchEmail.trimmed();
    if (trimmedString.isEmpty()) {
        deleteLater();
        return false;
    }

    Baloo::PIM::ContactCompleter com(trimmedString, 500);
    Q_EMIT emailsFound(com.complete());
    deleteLater();
    return true;
}

void BlackListBalooEmailSearchJob::setSearchEmail(const QString &searchEmail)
{
    mSearchEmail = searchEmail;
}

