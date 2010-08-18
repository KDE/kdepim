/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef AKONADI_CONTACT_CONTACTGROUPVIEWITEM_H
#define AKONADI_CONTACT_CONTACTGROUPVIEWITEM_H

#include "declarativeakonadiitem.h"

#include <akonadi/contact/contactgroupviewer.h>

namespace Akonadi {

namespace Contact {

/**
 * @short A wrapper class to make the 'removed' signal available.
 */
class ExtendedContactGroupViewer : public ContactGroupViewer
{
  Q_OBJECT

  public:
    ExtendedContactGroupViewer( QWidget *parent = 0 );
    ~ExtendedContactGroupViewer();

  Q_SIGNALS:
    void contactGroupRemoved();

  private:
    virtual void itemRemoved();
};

class ContactGroupViewItem : public DeclarativeAkonadiItem
{
  Q_OBJECT

  public:
    explicit ContactGroupViewItem( QDeclarativeItem *parent = 0 );

    qint64 itemId() const;
    void setItemId( qint64 id );

  Q_SIGNALS:
    void contactGroupRemoved();

  private:
    ExtendedContactGroupViewer *m_viewer;
};

}
}

#endif
