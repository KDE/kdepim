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
#include "Entry.h"
#include "Field.h"

Entry::Entry()
  : dirty_(false)
{
  // Empty.
}

Entry::Entry(const QDomElement & e)
  : dirty_(false)
{
  id_   = e.attribute("id");
  name_ = e.attribute("name");

  QDomNode n = e.firstChild();

  while (!n.isNull())
  {
    QDomElement child = n.toElement();

    if (!child.isNull())
    {
      if (child.tagName() == "kab:child-list")
      {
        QDomNode n2 = child;

        while (!n2.isNull())
        {
          QDomElement child2 = n2.toElement();

          if (!child2.isNull())
            if (child2.tagName() == "kab:child")
              memberList_.append(child2.nodeValue());
        }
      }
      else
      {
        fieldList_.append(Field(child));
      }
    }
  }
}

Entry::Entry(const QString & name)
  : dirty_(false),
    name_(name)
{
  // Empty.
}


Entry::~Entry()
{
  // Empty.
}

Entry::Entry(const Entry & e)
  : id_         (e.id_),
    name_       (e.name_),
    fieldList_  (e.fieldList_),
    memberList_ (e.memberList_)
{
}

  Entry &
Entry::operator = (const Entry & e)
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
Entry::operator == (const Entry & e) const
{
  return (id_ == e.id_);
}

  bool
Entry::isNull() const
{
  return id_.isNull();
}

  QString
Entry::id() const
{
  return id_;
}

  void
Entry::setID(const QString & id)
{
  id_ = id;
}

  QString
Entry::name() const
{
  return name_;
}

  void
Entry::setName(const QString & name)
{
  name_ = name;
  dirty_ = true;
}
  
  void
Entry::addField(const Field & f)
{
  fieldList_.append(f);
  dirty_ = true;
}

  bool
Entry::removeField(const QString & name)
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
Entry::contains(const QString & fieldName) const
{
  FieldList::ConstIterator it;

  for (it = fieldList_.begin(); it != fieldList_.end(); ++it)
    if ((*it).name() == fieldName)
      return true;

  return false;

}
  
  Field
Entry::field(const QString & name) const
{
  Field f;

  FieldList::ConstIterator it;

  for (it = fieldList_.begin(); it != fieldList_.end(); ++it)
    if ((*it).name() == name)
      return *it;

  return f;
}

  FieldList
Entry::fieldList() const
{
  return fieldList_;
}
  
  void
Entry::addMember(const QString & id)
{
  memberList_.append(id);
  dirty_ = true;
}

  bool
Entry::removeMember(const QString & id)
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
Entry::memberList() const
{
  return memberList_;
}

  QDomElement
Entry::toDomElement() const
{
  QDomElement e;

  e.setAttribute("id", id_);
  e.setAttribute("name", name_);

  FieldList::ConstIterator fit = fieldList_.begin();

  for (; fit != fieldList_.end(); ++fit)
    e.appendChild((*fit).toDomElement());
 
  if (!memberList_.isEmpty())
  {
    QDomElement memberListElement;
    memberListElement.setTagName("kab:child-list");

    QStringList::ConstIterator it(memberList_.begin());

    for (; it != memberList_.end(); ++it)
    {
      QDomElement memberElement;
      memberElement.setTagName("kab:child");
      memberElement.setNodeValue(*it);
      memberListElement.appendChild(memberElement);
    }

    e.appendChild(memberListElement);
  }

  return e;
}

  QDataStream &
operator << (QDataStream & str, const Entry & e)
{
  str <<  e.id_
      <<  e.name_
      <<  e.fieldList_
      <<  e.memberList_;

  return str;
}

  QDataStream &
operator >> (QDataStream & str, Entry & e)
{
  str >>  e.id_
      >>  e.name_
      >>  e.fieldList_
      >>  e.memberList_;

  e.dirty_ = false;

  return str;
}

  void
Entry::touch()
{
  dirty_ = true;
}

  void
Entry::replace(const QString & name, const QString & value)
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

