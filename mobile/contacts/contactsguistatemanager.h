/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#ifndef CONTACTSGUISTATEMANAGER_H
#define CONTACTSGUISTATEMANAGER_H

#include "../lib/guistatemanager.h"

class ContactsGuiStateManager : public GuiStateManager
{
  Q_OBJECT

  Q_PROPERTY( bool inViewContactState READ inViewContactState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inViewContactGroupState READ inViewContactGroupState NOTIFY guiStateChanged )

  Q_ENUMS( GuiState )

  public:
    enum GuiState {
      ViewContactState = GuiStateManager::UserState,
      ViewContactGroupState
    };

    /**
     * Returns whether the current state is the view contact state.
     */
    bool inViewContactState() const;

    /**
     * Returns whether the current state is the view contact group state.
     */
    bool inViewContactGroupState() const;

  Q_SIGNALS:
    void guiStateChanged();

  protected:
    virtual void emitChangedSignal();
};

#endif
