/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "testpersonimpl.h"
#include "person.h"
#include "personimpl.h"
#include "tools.h"
#include <QList>
#include <QString>
#include <QStringList>

using Syndication::Person;
using Syndication::PersonPtr;
using Syndication::PersonImpl;

void TestPersonImpl::fromString()
{
    QStringList s;
    QList<PersonPtr> p;

    
    
    s.append(QString());
    p.append(PersonPtr(new PersonImpl(QString(), QString(), QString())));
    
    s.append("");
    p.append(PersonPtr(new PersonImpl(QString(), QString(), QString())));
    
    s.append("foo@bar.com");
    p.append(PersonPtr(new PersonImpl(QString(), QString(), "foo@bar.com")));
    
    s.append("<foo@bar.com>");
    p.append(PersonPtr(new PersonImpl(QString(), QString(), "foo@bar.com")));
    
    s.append("Foo");
    p.append(PersonPtr(new PersonImpl("Foo", QString(), QString())));
    
    s.append("Foo M. Bar");
    p.append(PersonPtr(new PersonImpl("Foo M. Bar", QString(), QString())));
    
    s.append("Foo <foo@bar.com>");
    p.append(PersonPtr(new PersonImpl("Foo", QString(), "foo@bar.com")));
    
    s.append("Foo Bar <foo@bar.com>");
    p.append(PersonPtr(new PersonImpl("Foo Bar", QString(), "foo@bar.com")));

    s.append("<foo@bar.com> (Foo Bar)");
    p.append(PersonPtr(new PersonImpl("Foo Bar", QString(), "foo@bar.com")));
    
    s.append("OnAhlmann (mailto:&amp;#111;&amp;#110;&amp;#97;&amp;#104;&amp;#108;&amp;#109;&amp;#97;&amp;#110;&amp;#110;&amp;#64;&amp;#103;&amp;#109;&amp;#97;&amp;#105;&amp;#108;&amp;#46;&amp;#99;&amp;#111;&amp;#109;)");
    p.append(PersonPtr(new PersonImpl("OnAhlmann", QString(), "onahlmann@gmail.com")));
    
    QList<PersonPtr> q;
    
    QStringList::ConstIterator it = s.begin();
    QStringList::ConstIterator end = s.end();
    QList<PersonPtr>::ConstIterator pit = p.begin();
    
    while (it != end)
    {
        PersonPtr q(Syndication::personFromString(*it));
        QCOMPARE(q->debugInfo(), (*pit)->debugInfo());
        ++it;
        ++pit;
    }
}

QTEST_MAIN(TestPersonImpl)

#include "testpersonimpl.moc"
