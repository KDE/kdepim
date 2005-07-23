/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSYNC_PROFILECHECKITEM_H
#define KSYNC_PROFILECHECKITEM_H

#include <qlistview.h>

#include "actionpartservice.h"

namespace KSync {

// FIXME: Rename to ActionPartItem
class ProfileCheckItem : public QListViewItem
{
  public:
    ProfileCheckItem( QListView *parent, const ActionPartService & );
    ProfileCheckItem( QListView *parent, QListViewItem *after,
                      const ActionPartService & );
    ~ProfileCheckItem();
    
    ActionPartService actionPart() const;

  protected:
    void init();

  private:
    ActionPartService m_part;
};

}

#endif
