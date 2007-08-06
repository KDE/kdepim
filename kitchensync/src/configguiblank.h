/*
    This file is part of KitchenSync.

    Copyright (c) 2006 Daniel Gollub  <dgollub@suse.de>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/
#ifndef CONFIGGUIBLANK_H
#define CONFIGGUIBLANK_H

#include "configgui.h"

class ConfigGuiBlank : public ConfigGui
{
  public:
    ConfigGuiBlank( const QSync::Member &member, QWidget *parent = 0 );

    void load( const QString &xml );
    QString save() const;
};

#endif
