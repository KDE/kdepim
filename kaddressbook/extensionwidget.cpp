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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "configurewidget.h"

#include "extensionwidget.h"

ExtensionWidget::ExtensionWidget( ViewManager *vm, QWidget *parent,
                                    const char *name )
  : QWidget( parent, name ), mViewManager( vm )
{
}

ExtensionWidget::~ExtensionWidget()
{
}

KABC::AddressBook *ExtensionWidget::addressBook() const
{
  return mViewManager->addressBook();
}

ViewManager *ExtensionWidget::viewManager() const
{
  return mViewManager;
}

bool ExtensionWidget::addresseesSelected() const
{
  return mViewManager->selectedUids().count() != 0;
}

KABC::Addressee::List ExtensionWidget::selectedAddressees()
{
  KABC::Addressee::List list;

  QStringList uids = mViewManager->selectedUids();
  QStringList::Iterator it;
  for ( it = uids.begin(); it != uids.end(); ++it )
    list.append( addressBook()->findByUid( *it ) );

  return list;
}

void ExtensionWidget::addresseeSelectionChanged()
{
  // do nothing
}

QString ExtensionWidget::title() const
{
  return "";
}

QString ExtensionWidget::identifier() const
{
  return "empty_widget";
}

ConfigureWidget *ExtensionFactory::configureWidget( ViewManager*, QWidget*,
                                                    const char* )
{
  return 0;
}

#include "extensionwidget.moc"
