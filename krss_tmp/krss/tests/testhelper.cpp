/*
 * This file is part of the krss library
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
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
 *
 */

#include "testhelper.h"
#include "helper_p.h"

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>

#include <qtest_kde.h>

void TestHelper::testAttributeSerialization()
{
    QHash<QString,QString> p;
    QCOMPARE( KRss::decodeProperties( QByteArray() ), p );
    QCOMPARE( KRss::encodeProperties( p ), QByteArray() );
    p.insert( QLatin1String("Key"), QLatin1String("Value") );
    p.insert( QLatin1String("\\=;;;\\;"), QLatin1String("dsfds ; ;\\\\;\\\\\\=") );
    p.insert( QLatin1String("\\\\=;;;\\;"), QLatin1String("dsfds ; ;\\\\;dsf\\\\\\=") );
    const QByteArray serialized = KRss::encodeProperties( p );
    const QHash<QString, QString> read = KRss::decodeProperties( serialized );
    QCOMPARE( read, p );
    const QStringList emptyList;
    QCOMPARE( KRss::decodeStringList( QByteArray() ), emptyList );
    QCOMPARE( KRss::encodeStringList( emptyList ), QByteArray() );
    const QStringList origKeys = p.keys();
    const QByteArray serializedKeys = KRss::encodeStringList(  origKeys );
    const QStringList readKeys = KRss::decodeStringList( serializedKeys );
    QCOMPARE( readKeys, origKeys );
}

QTEST_KDEMAIN( TestHelper, NoGUI )

#include "testhelper.moc"
