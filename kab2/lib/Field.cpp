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

// Local includes
#include "Field.h"

Field::Field()
{
  // Empty.
}

Field::Field(const QDomElement & e)
{
  name_ = e.attribute("name");

  QString mt(e.attribute("mimetype"));

  if (!mt.isEmpty())
  {
    int sep = mt.find('/');

    if (-1 == sep)
      type_ = mt;

    else
    {
      type_ = mt.left(sep);
      subType_ = mt.mid(sep + 1);
    }
  }

//  value_ = decode it !
}

Field::Field(const QString & name)
  : name_(name)
{
  // Empty.
}

Field::Field(const QString & name, const QString & value)
  : name_(name)
{
  setValue(value);
}


Field::~Field()
{
  // Empty.
}

Field::Field(const Field & f)
  : name_     (f.name_),
    type_     (f.type_),
    subType_  (f.subType_),
    value_    (f.value_)
{
  // Empty.
}

  Field &
Field::operator = (const Field & f)
{
  if (this == &f) // Avoid a = a.
    return *this;

  name_     = f.name_;
  type_     = f.type_;
  subType_  = f.subType_;
  value_    = f.value_;

  return *this;
}

  bool
Field::operator == (const Field & f) const
{
  return
    ( name_     == f.name_    &&
      type_     == f.type_    &&
      subType_  == f.subType_ &&
      value_    == f.value_   );
}

  bool
Field::isNull() const
{
  return name_.isNull();
}

  QString
Field::name() const
{
  return name_;
}

  QString
Field::type() const
{
  return type_;
}

  QString
Field::subType() const
{
  return subType_;
}

  QByteArray
Field::value() const
{
  return value_;
}

  QString
Field::stringValue() const
{
  QString ret;

  if
    (
     (type_.isEmpty()     || (type_ == "text")      ) &&
     (subType_.isEmpty()  || (subType_ == "UCS-2")  )
    )
  {
    QDataStream str(value_, IO_ReadOnly);
    str >> ret;
  }

  return ret;
}

  void
Field::setName(const QString & s)
{
  name_ = s;
}

  void
Field::setType(const QString & s)
{
  type_ = s;
}

  void
Field::setSubType(const QString & s)
{
  subType_ = s;
}

  void
Field::setValue(const QByteArray & a)
{
  value_ = a;
}

  void
Field::setValue(const QString & s)
{
  QDataStream str(value_, IO_WriteOnly);
  str << s;
}

  void
Field::insertInDomTree(QDomNode & parent, QDomDocument & parentDoc) const
{
  QDomElement e = parentDoc.createElement(name_);

  // FIXME - need to encode other data types !
  e.appendChild(parentDoc.createTextNode(stringValue()));

  parent.appendChild(e);
}

  QDataStream &
operator << (QDataStream & str, const Field & f)
{
  str << f.name_ << f.type_ << f.subType_ << f.value_;

  return str;
}

  QDataStream &
operator >> (QDataStream & str, Field & f)
{
  str >> f.name_ >> f.type_ >> f.subType_ >> f.value_;

  return str;
}

