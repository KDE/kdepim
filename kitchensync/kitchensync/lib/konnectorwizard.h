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
#ifndef KSYNC_KONNECTOR_WIZARD_H
#define KSYNC_KONNECTOR_WIZARD_H

#include <qmap.h> //qt

#include <kwizard.h> //kde

#include <kdevice.h> // libkonnector

#include <konnectorprofile.h> //kitchensyncui

class KonnectorProfileWizardIntro;
class KonnectorWizardOutro;
namespace KSync{
    class KonnectorManager;
    class ConfigWidget;

    /**
     * The KonnectorWizzard is a KWizard where
     * you can configure a KonnectorProfile
     * including setting the name and creating 
     * a Kapabilities
     * @short a small wizard to create a KonnectorProfile
     */
    class KonnectorWizard : public KWizard {
        Q_OBJECT
    public:
    
	/**
	 * Constructor for creating a new KonnectorProfile
	 * @param manager the KonnectorManager to be used
	 */
        KonnectorWizard( KonnectorManager* manager);
	
	/**
	 * Constructor to edit a KonnectorProfile
	 * @param manager the KonnectorManager to be used
	 * @param prof The KonnectorProfile
	 */
        KonnectorWizard( KonnectorManager* manager, const KonnectorProfile& prof);
        ~KonnectorWizard();
	
	/**
	 * @return the edited KonnectorProfile
	 */
        KonnectorProfile profile() const;
    private:
        void initUI();
        Device byString( const QString&  )const;
        KonnectorManager* m_manager;
        KonnectorProfileWizardIntro *m_intro;
        KonnectorWizardOutro *m_outro;
        ConfigWidget* m_conf;
        QMap<QString, Device> m_devices;
        QString m_current;
	bool m_isEdit;
	Kapabilities m_kaps;

      private slots:
        void slotKonChanged( const QString& );
    };

}


#endif
