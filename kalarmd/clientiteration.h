/*
    KDE Alarm Daemon GUI.

    This file is part of the GUI interface for the KDE alarm daemon.
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>
    Based on the original, (c) 1998, 1999 Preston Brown

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
#ifndef CLIENTITERATION_H
#define CLIENTITERATION_H

#include "calclient.h"

// The ClientIteration class gives secure public access to AlarmGui::mClients
class ClientIteration
{
  public:
    ClientIteration(ClientMap& c)     : clients(c) { iter = clients.begin(); }
    bool           ok() const         { return iter != clients.end(); }
    bool           next()             { return ++iter != clients.end(); }
    const QString& appName() const    { return iter.key(); }
    const QString& title() const      { return iter.data().title; }
    int            menuIndex() const  { return iter.data().menuIndex; }
    void           menuIndex(int n)   { iter.data().menuIndex = n; }
  private:
    ClientMap&          clients;
    ClientMap::Iterator iter;
};

#endif
