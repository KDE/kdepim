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

// Qt includes
#include <qcstring.h>

// Local includes
#include "Entity.h"
#include "Field.h"

Entity::Entity()
  : dirty_(false)
{
  // Empty.
}

Entity::Entity(const QString & name)
  : dirty_(false),
    name_(name)
{
  // Empty.
}


Entity::~Entity()
{
  // Empty.
}

Entity::Entity(const Entity & e)
  : id_         (e.id_),
    name_       (e.name_),
    fieldList_  (e.fieldList_),
    memberList_ (e.memberList_)
{
}

  Entity &
Entity::operator = (const Entity & e)
{
  if (this == &e) // Avoid a = a.
    return *this;

  id_         = e.id_;
  name_       = e.name_;
  fieldList_  = e.fieldList_;
  memberList_ = e.memberList_;

  return *this;
}

  bool
Entity::operator == (const Entity & e) const
{
  return (id_ == e.id_);
}

  bool
Entity::isNull() const
{
  return id_.isNull();
}

  QString
Entity::id() const
{
  return id_;
}

  void
Entity::setID(const QString & id)
{
  id_ = id;
}

  QString
Entity::name() const
{
  return name_;
}

  void
Entity::setName(const QString & name)
{
  name_ = name;
  dirty_ = true;
}
  
  void
Entity::addField(const Field & f)
{
  fieldList_.append(f);
  dirty_ = true;
}

  bool
Entity::removeField(const QString & name)
{
  FieldList::Iterator it;

  for (it = fieldList_.begin(); it != fieldList_.end(); ++it)

    if ((*it).name() == name) {
      fieldList_.remove(it);
      dirty_ = true;
      return true;
    }

  return false;
}

  bool
Entity::contains(const QString & fieldName) const
{
  FieldList::ConstIterator it;

  for (it = fieldList_.begin(); it != fieldList_.end(); ++it)
    if ((*it).name() == fieldName)
      return true;

  return false;

}
  
  Field
Entity::field(const QString & name) const
{
  Field f;

  FieldList::ConstIterator it;

  for (it = fieldList_.begin(); it != fieldList_.end(); ++it)
    if ((*it).name() == name)
      return *it;

  return f;
}

  FieldList
Entity::fieldList() const
{
  return fieldList_;
}
  
  void
Entity::addMember(const QString & id)
{
  memberList_.append(id);
  dirty_ = true;
}

  bool
Entity::removeMember(const QString & id)
{
  QStringList::Iterator it;

  for (it = memberList_.begin(); it != memberList_.end(); ++it)
    if (*it == id) {
      memberList_.remove(it);
      dirty_ = true;
      return true;
    }

  return false;
}

  QStringList
Entity::memberList() const
{
  return memberList_;
}

  QDataStream &
operator << (QDataStream & str, const Entity & e)
{
  str <<  e.id_
      <<  e.name_
      <<  e.fieldList_
      <<  e.memberList_;
}

  QDataStream &
operator >> (QDataStream & str, Entity & e)
{
  str >>  e.id_
      >>  e.name_
      >>  e.fieldList_
      >>  e.memberList_;

  e.dirty_ = false;
}

  void
Entity::touch()
{
  dirty_ = true;
}

  void
Entity::replace(const QString & name, const QString & value)
{
  removeField(name);
  Field f;
  f.setName(name);
  f.setType("text");
  f.setSubType("unicode");
  QByteArray a;
  QDataStream str(a, IO_WriteOnly);
  str << value;
  f.setValue(a);
  addField(f);
}

