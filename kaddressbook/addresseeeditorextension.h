/*
    This file is part of KAddressBook.
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#ifndef ADDRESSEEEDITOREXTENSION_H
#define ADDRESSEEEDITOREXTENSION_H

#include "addresseeeditorbase.h"
#include "extensionwidget.h"

class AddresseeEditorExtension : public KAB::ExtensionWidget
{
  Q_OBJECT

  public:
    AddresseeEditorExtension( KAB::Core *core, QWidget *parent );
    ~AddresseeEditorExtension();

    /**
      This method is called whenever the selection in the view changed.
     */
    virtual void contactsSelectionChanged();

    /**
      This method should be reimplemented and return the i18ned title of this
      widget.
     */
    virtual QString title() const;

    /**
      This method should be reimplemented and return a unique identifier.
     */
    virtual QString identifier() const;
  private slots:
    void emitModifiedAddresses();
    QSize minimumSizeHint() const;

  private:
    AddresseeEditorBase *mAddresseeEditor;
    bool mDirty;
    KABC::Addressee::List addressees;
};

#endif
