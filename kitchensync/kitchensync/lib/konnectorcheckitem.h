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
#ifndef KSYNC_KONNECTORCHECKITEM_H
#define KSYNC_KONNECTORCHECKITEM_H

#include <qlistview.h>

#include "konnectorprofile.h"

namespace KSync {

/**
 * A KonnectorCheckItem takes a KonnectorProfile
 * it has a QCheckBox to signalize if it has to be loaded
 * or unloaded. The differences between KonnectorProfile::konnector()
 * the state of this item decides if it has to be loaded or unloaded
 */
class KonnectorCheckItem : public QCheckListItem
{
  public:
    /**
     * c'tor the Parent and the Profile
     */
    KonnectorCheckItem( QListView *parent, const KonnectorProfile &prof );
    ~KonnectorCheckItem();

    /**
     * Retur the profile
     * @return the KonnectorProfile
     */	
    KonnectorProfile profile() const;

    /**
     * @return if it has to be loaded
     *
     */
    bool load() const;

    /**
     * @return it has to be unloaded
     **/
    bool unload() const;

    /**
     * @return if the konnector currently is loaded
     */
    bool isLoaded() const;

    /**
     * @return if bool was edited
     */
    bool wasEdited() const;

     /**
      * Set when the KonnectorProfile
      * was edited
      * @param b if it was edited
      */
    void setEdited( bool b );

    /**
     * Set the KonnectorProfile
     * @param prof the konnector profile
     */
    void setProfile( const KonnectorProfile &prof );

  private:
    KonnectorProfile m_prof;
    bool m_edit :1 ;

};

}

#endif
