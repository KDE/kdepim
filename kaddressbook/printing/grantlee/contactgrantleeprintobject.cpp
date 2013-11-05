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

#include "contactgrantleeprintobject.h"

using namespace KABPrinting;

ContactGrantleePrintObject::ContactGrantleePrintObject(const KABC::Addressee &address, QObject *parent)
    : QObject(parent),
      mAddress(address)
{
}

ContactGrantleePrintObject::~ContactGrantleePrintObject()
{

}

QString ContactGrantleePrintObject::realName() const
{
    return mAddress.realName();
}

QString ContactGrantleePrintObject::formattedName() const
{
    return mAddress.formattedName();
}

QString ContactGrantleePrintObject::prefix() const
{
    return mAddress.prefix();
}

QString ContactGrantleePrintObject::givenName() const
{
    return mAddress.givenName();
}

QString ContactGrantleePrintObject::additionalName() const
{
    return mAddress.additionalName();
}

QString ContactGrantleePrintObject::familyName() const
{
    return mAddress.familyName();
}

QString ContactGrantleePrintObject::suffix() const
{
    return mAddress.suffix();
}

QString ContactGrantleePrintObject::nickName() const
{
    return mAddress.nickName();
}

QStringList ContactGrantleePrintObject::emails() const
{
    QStringList emails;
    Q_FOREACH ( const QString &email, mAddress.emails() ) {
        const QString fullEmail = QString::fromLatin1( KUrl::toPercentEncoding( mAddress.fullEmail( email ) ) );

        const QString url = QString::fromLatin1( "<a href=\"mailto:%1\">%2</a>" )
                .arg( fullEmail, email );
        emails << url;
    }
    return emails;
}

QString ContactGrantleePrintObject::organization() const
{
    return mAddress.organization();
}

QString ContactGrantleePrintObject::note() const
{
    return mAddress.note().replace(QLatin1Char('\n'), QLatin1String("<br>"));
}

QString ContactGrantleePrintObject::webPage() const
{
    return mAddress.url().prettyUrl();
}

QString ContactGrantleePrintObject::title() const
{
    return mAddress.title();
}

QString ContactGrantleePrintObject::preferredEmail() const
{
    return mAddress.preferredEmail();
}

QString ContactGrantleePrintObject::role() const
{
    return mAddress.role();
}

