/*
    KAddressBook version 2
    
    Copyright (C) 1999 The KDE PIM Team <kde-pim@kde.org>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef KADDRESSBOOK_ENTITY_H
#define KADDRESSBOOK_ENTITY_H

// Qt includes
#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qcstring.h>
#include <qstringlist.h>
#include <qdom.h>

// Local includes
#include "Field.h"

class Entry
{
  public:

    Entry();
    Entry(const QDomElement &);
    Entry(const QString & name);
    Entry(const Entry &);
    virtual ~Entry();

    Entry & operator = (const Entry &);
    bool operator == (const Entry &) const;

    QDomElement toDomElement() const;

    virtual QString id() const;
    virtual void setID(const QString &);
    virtual QString name() const;

    virtual void setName(const QString & name);
    
    virtual void addField(const Field &);
    virtual bool removeField(const QString & name);
    
    virtual Field     field(const QString & name) const;
    virtual FieldList fieldList() const;
    
    virtual void addMember(const QString & id);
    virtual bool removeMember(const QString & id);

    virtual QStringList memberList() const;

    virtual bool contains(const QString & fieldName) const;

    void replace(const QString & name, const QByteArray & value);
    void replace(const QString & name, const QString & value);

    void touch();

    friend QDataStream & operator << (QDataStream &, const Entry &);
    friend QDataStream & operator >> (QDataStream &, Entry &);

    bool isNull() const;

  private:

    bool dirty_;

    QString id_, name_;
    FieldList fieldList_;
    QStringList memberList_;
};

typedef QValueList<Entry> EntryList;

#endif
