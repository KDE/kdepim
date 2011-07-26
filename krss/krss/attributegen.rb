#!/usr/bin/env ruby

# like SomeAttributes
name = ARGV[0]
name = name[0, 1].upcase + name[1, name.size]
# like someAttributes, used for the m_* variable
lname = name[0, 1].downcase + name[1, name.size]
# like SomeAttribute, used in add*() and remove*()
sname = name[0, name.size - 1]
# like someAttribute, used in add*() and remove*() as an argument
slname = sname[0, 1].downcase + sname[1, sname.size]

header = File.new("#{name.downcase}collectionattribute.h", "w")
header.puts \
"/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AKONADI_#{name.upcase}COLLECTION_ATTRIBUTE
#define AKONADI_#{name.upcase}COLLECTION_ATTRIBUTE

#include <akonadi/attribute.h>

#include <QStringList>

class #{name}CollectionAttribute : public Akonadi::Attribute
{
public:

        #{name}CollectionAttribute( const QStringList &#{lname} = QStringList() );
        QByteArray type() const;
        #{name}CollectionAttribute* clone() const;
        QByteArray serialized() const;
        void deserialize( const QByteArray &data );

        QStringList #{lname}() const;
        void set#{name}( const QStringList &#{lname} );
        void add#{sname}( const QString &#{slname} );
        void remove#{sname}( const QString &#{slname} );

private:

        QStringList m_#{lname};
};

#endif /* AKONADI_#{name.upcase}COLLECTION_ATTRIBUTE */
"
header.close

source = File.new("#{name.downcase}collectionattribute.cpp", "w")
source.puts \
"/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include \"#{name.downcase}collectionattribute.h\"

#{name}CollectionAttribute::#{name}CollectionAttribute( const QStringList &#{lname} )
        : Attribute(), m_#{lname}( #{lname} )
{
}

QByteArray #{name}CollectionAttribute::type() const
{
        return \"#{name}\";
}

#{name}CollectionAttribute* #{name}CollectionAttribute::clone() const
{
        #{name}CollectionAttribute *attr = new #{name}CollectionAttribute( m_#{lname} );
        return attr;
}

QByteArray #{name}CollectionAttribute::serialized() const
{
        return m_#{lname}.join( \";\" ).toUtf8();
}

void #{name}CollectionAttribute::deserialize( const QByteArray &data )
{
        if ( data.isEmpty() )
                return;

        // so ugly, am i missing something?
        m_#{lname} = QString::fromUtf8( data.constData(), data.size() ).split( ';' );
}

QStringList #{name}CollectionAttribute::#{lname}() const
{
        return m_#{lname};
}

void #{name}CollectionAttribute::set#{name}( const QStringList &#{lname} )
{
        m_#{lname} = #{lname};
}

void #{name}CollectionAttribute::add#{sname}( const QString &#{slname} )
{
        if ( !m_#{lname}.contains( #{slname} ) )
                m_#{lname} << #{slname};
}

void #{name}CollectionAttribute::remove#{sname}( const QString &#{slname} )
{
        // add#{sname} ensure the list contains no duplicates
        m_#{lname}.removeOne( #{slname} );
}
"
source.close
