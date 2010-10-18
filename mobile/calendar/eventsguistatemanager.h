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

#ifndef EVENTSGUISTATEMANAGER_H
#define EVENTSGUISTATEMANAGER_H

#include "../lib/guistatemanager.h"

class EventsGuiStateManager : public GuiStateManager
{
  Q_OBJECT

  Q_PROPERTY( bool inViewDayState READ inViewDayState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inViewWeekState READ inViewWeekState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inViewMonthState READ inViewMonthState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inViewEventListState READ inViewEventListState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inViewTimelineState READ inViewTimelineState NOTIFY guiStateChanged )

  Q_ENUMS( GuiState )

  public:
    enum GuiState {
      ViewDayState = GuiStateManager::UserState,
      ViewWeekState,
      ViewMonthState,
      ViewEventListState,
      ViewTimelineState
    };

    /**
     * Returns whether the current state is the view day state.
     */
    bool inViewDayState() const;

    /**
     * Returns whether the current state is the view week state.
     */
    bool inViewWeekState() const;

    /**
     * Returns whether the current state is the view month state.
     */
    bool inViewMonthState() const;

    /**
     * Returns whether the current state is the view event list state.
     */
    bool inViewEventListState() const;

    /**
     * Returns whether the current state is the view timeline state.
     */
    bool inViewTimelineState() const;

  Q_SIGNALS:
    void guiStateChanged();

  protected:
    virtual void emitChangedSignal();
};

#endif
