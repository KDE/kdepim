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

#ifndef KADDRESSBOOK_FIELD_H
#define KADDRESSBOOK_FIELD_H

// Qt includes
#include <qstring.h>
#include <qvaluelist.h>
#include <qdict.h>

typedef QDict<QString> StringDict;
typedef QDictIterator<QString> StringDictIterator;

class Field
{
  public:
    
    Field();
    Field(const QString & name);
    Field(const QString & name, const QString & value);
    Field(const Field &);
    virtual ~Field();

    Field & operator = (const Field &);
    bool operator == (const Field &) const;
    
    virtual QString     name()    const;
    virtual QString     type()    const;
    virtual QString     subType() const;
    virtual QByteArray  value()   const;
    virtual QString     stringValue() const;

    virtual void setName    (const QString &);
    virtual void setType    (const QString &);
    virtual void setSubType (const QString &);
    virtual void setValue   (const QByteArray &);
    virtual void setValue   (const QString &);
    
    friend QDataStream & operator << (QDataStream &, const Field &);
    friend QDataStream & operator >> (QDataStream &, Field &);

    bool isNull() const;

  private:

    QString name_, type_, subType_;
    QByteArray value_;
};

typedef QValueList<Field> FieldList;

#endif
