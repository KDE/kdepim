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
#ifndef KSYNC_KONNECTOR_DIALOG_H_CONFIG
#define KSYNC_KONNECTOR_DIALOG_H_CONFIG

#include <qptrlist.h>

#include <kdialogbase.h>

#include "konnectorprofile.h"
#include "konnectorcheckitem.h"

class KonnectorProfileListBase;
namespace KSync {
    class KonnectorManager;
    /**
     * KonnectorDialog is the configure dialog to for the
     * user to load, unload, add and remove KonnectorProfiles
     * It starts the Wizzard and gives informations about which
     * konnectors to load and unload
     */
    class KonnectorDialog : public KDialogBase {
        Q_OBJECT
    public:
        /**
         * it's modal dialog and don't need a parent
	 * @short a simple c'tor
	 * @param lst A List of KonnectorProfiles
	 * @param man The KonnectorManager to be used
         */
        KonnectorDialog( const KonnectorProfile::ValueList& lst , KonnectorManager* man);
        ~KonnectorDialog();

        /**
         * @return devices which needs to be unloaded
	 *
         */
        KonnectorProfile::ValueList toUnload() const;

        /**
         * @return devices which need to be loaded
         */
        KonnectorProfile::ValueList toLoad() const;

        /**
         * @return all devices
         */
        KonnectorProfile::ValueList devices() const;

        /**
         * guess it! Wrong! The removed devices/profiles
	 * @return the removed items
         */
        KonnectorProfile::ValueList removed() const;
	
	/**
	 * the edited devices...which are also loaded...
	 * @return the edited devices
	 */
	KonnectorProfile::ValueList edited()const;

    protected slots:
        virtual void slotRemove();
        virtual void slotAdd();
	virtual void slotEdit();
    private:
        KonnectorProfile::ValueList m_list;
        KonnectorManager* m_manager;
        /** KonnectoCheckItem -> KonnectorProfile::ValueList */
        QPtrList<KonnectorCheckItem> list2list()const;
        KonnectorProfile::ValueList list()const;
        void initListView();
        KonnectorProfileListBase *m_base;

    };
}

#endif
