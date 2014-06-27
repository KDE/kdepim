/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Based in kmail/recipientseditor.h/cpp
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/


#include "recipient.h"

#include <klocale.h>

using namespace KPIM;
using namespace MessageComposer;

Recipient::Recipient( const QString &email, Recipient::Type type )
    : mEmail( email ), mType( type )
{
}

void Recipient::setType( Type type )
{
    mType = type;
}

Recipient::Type Recipient::type() const
{
    return mType;
}

void Recipient::setEmail( const QString &email )
{
    mEmail = email;
}

QString Recipient::email() const
{
    return mEmail;
}

void Recipient::setName( const QString &name )
{
    mName = name;
}

QString Recipient::name() const
{
    return mName;
}

bool Recipient::isEmpty() const
{
    return mEmail.isEmpty();
}

void Recipient::clear()
{
    mEmail.clear();
    mType = Recipient::To;
}

int Recipient::typeToId( Recipient::Type type )
{
    return static_cast<int>( type );
}

Recipient::Type Recipient::idToType( int id )
{
    return static_cast<Type>( id );
}

QString Recipient::typeLabel() const
{
    return typeLabel( mType );
}

QString Recipient::typeLabel( Recipient::Type type )
{
    switch( type ) {
    case To:
        return i18nc("@label:listbox Recipient of an email message.", "To");
    case Cc:
        return i18nc("@label:listbox Carbon Copy recipient of an email message.", "CC");
    case Bcc:
        return i18nc("@label:listbox Blind carbon copy recipient of an email message.", "BCC");
    case Undefined:
        break;
    }

    return i18nc("@label:listbox", "<placeholder>Undefined Recipient Type</placeholder>");
}

QStringList Recipient::allTypeLabels()
{
    QStringList types;
    types.append( typeLabel( To ) );
    types.append( typeLabel( Cc ) );
    types.append( typeLabel( Bcc ) );
    return types;
}
