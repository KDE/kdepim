
#ifndef KSYNC_KONNECTOR_CHECK_ITEM_H
#define KSYNC_KONNECTOR_CHECK_ITEM_H

#include <qlistview.h>

#include "konnectorprofile.h"

namespace KSync {

    /**
     * A KonnectorCheckItem takes a KonnectorProfile
     * it has a QCheckBox to signalize if it has to be loaded
     * or unloaded. The differences between KonnectorProfile::udi()
     * the state of this item decides if it has to be loaded or unloaded
     */
    class KonnectorCheckItem : public QCheckListItem {
    public:
        /**
         * c'tor the Parent and the Profile
         */
        KonnectorCheckItem( QListView* parent, const KonnectorProfile& prof );
        ~KonnectorCheckItem();

    
	/**
	 * Retur the profile
	 * @return the KonnectorProfile
	 */	
        KonnectorProfile profile()const;

        /**
         * @return if it has to be loaded
	 *
         */
        bool load()const;

        /**
         * @return it has to be unloaded
         **/
        bool unload()const;

        /**
         * @return if the konnector currently is loaded
         */
        bool isLoaded()const;
	
	/**
	 * @return if bool was edited
	 */
	bool wasEdited()const;
	 
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
	void setProfile( const KonnectorProfile& prof);
    private:
        KonnectorProfile m_prof;
	bool m_edit :1 ;

    };

};

#endif
