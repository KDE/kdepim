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

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef MESSAGECOMPOSER_RECIPIENT_H
#define MESSAGECOMPOSER_RECIPIENT_H

#include "messagecomposer_export.h"

#include <libkdepim/multiplyingline/multiplyingline.h>

#include <QString>
#include <QSharedPointer>
namespace MessageComposer {

/** Represents a mail recipient. */
class MESSAGECOMPOSER_EXPORT Recipient : public KPIM::MultiplyingLineData
{
public:
    typedef QSharedPointer<Recipient> Ptr;
    typedef QList<Recipient::Ptr> List;

    enum Type { To, Cc, Bcc, Undefined };

    Recipient( const QString &email = QString(), Type type = To ); //krazy:exclude=explicit

    void setType( Type type );
    Type type() const;

    void setEmail( const QString & email );
    QString email() const;

    void setName ( const QString &name );
    QString name() const;

    virtual bool isEmpty() const;
    virtual void clear();

    static int typeToId( Type type);
    static Type idToType( int id );

    QString typeLabel() const;
    static QString typeLabel( Type type );
    static QStringList allTypeLabels();

private:
    QString mEmail;
    QString mName;
    Type mType;
};

}

#endif
