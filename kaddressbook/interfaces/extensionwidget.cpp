/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "core.h"

#include "extensionwidget.h"

using namespace KAB;

ExtensionWidget::ExtensionWidget( Core *core, QWidget *parent,
                                  const char *name )
  : QWidget( parent, name ), mCore( core )
{
}

ExtensionWidget::~ExtensionWidget()
{
}

KAB::Core *ExtensionWidget::core() const
{
  return mCore;
}

bool ExtensionWidget::contactsSelected() const
{
  return mCore->selectedUIDs().count() != 0;
}

KABC::Addressee::List ExtensionWidget::selectedContacts()
{
  KABC::Addressee::List list;

  const QStringList uids = mCore->selectedUIDs();
  QStringList::ConstIterator it;
  for ( it = uids.begin(); it != uids.end(); ++it )
    list.append( mCore->addressBook()->findByUid( *it ) );

  return list;
}

void ExtensionWidget::contactsSelectionChanged()
{
  // do nothing
}

#include "extensionwidget.moc"
