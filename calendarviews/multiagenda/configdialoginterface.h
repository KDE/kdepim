/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Sergio Martins <sergio.martins@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef EVENTVIEWS_CONFIGDIALOGINTERFACE_H
#define EVENTVIEWS_CONFIGDIALOGINTERFACE_H

class KCheckableProxyModel;

namespace EventViews {

class ConfigDialogInterface
{
  public:
    virtual ~ConfigDialogInterface() { };

    virtual int numberOfColumns() const = 0;
    virtual bool useCustomColumns() const = 0;
    virtual QString columnTitle( int column ) const = 0;

    virtual KCheckableProxyModel *takeSelectionModel( int column ) = 0;
};

}

#endif
