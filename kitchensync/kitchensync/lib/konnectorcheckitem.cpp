/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "konnectorcheckitem.h"

using namespace KSync;

KonnectorCheckItem::KonnectorCheckItem( QListView* parent,  const KonnectorProfile& prof )
    : QCheckListItem( parent, QString::null, CheckBox),
      m_prof( prof )
{
    setText(0, "");
    setText(1, prof.name() );
    setOn( prof.konnector() );
    m_edit = false;
}

KonnectorCheckItem::~KonnectorCheckItem()
{
}

KonnectorProfile KonnectorCheckItem::profile() const
{
    return m_prof;
}

bool KonnectorCheckItem::load() const
{
    bool load = false;
    if ( !m_prof.konnector() &&  isEnabled() ) // not loaded but it's marked as loaded
        load = true;

    return load;
}

bool KonnectorCheckItem::unload() const
{
    bool load = false;
    if ( m_prof.konnector() && !isEnabled() ) // loaded but not marked as loaded unload it now
        load = true;

    return load;
}

bool KonnectorCheckItem::isLoaded() const
{
    return ( m_prof.konnector() );
}

bool KonnectorCheckItem::wasEdited() const
{
    return m_edit;
}

void KonnectorCheckItem::setEdited( bool b )
{
    m_edit = b;
}

void KonnectorCheckItem::setProfile( const KonnectorProfile &prof )
{
    m_prof = prof;
}
