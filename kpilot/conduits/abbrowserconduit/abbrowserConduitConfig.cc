#include "abbrowserConduitConfig.h"
#include <kconfig.h>
#include "abbrowser-conduit.h"
#include <kcombobox.h>
#include <qcheckbox.h>

const QString AbbrowserConduitConfig::SMART_MERGE_ENTRY = "SmartMerge";
const QString
AbbrowserConduitConfig::CONFLICT_RESOLV_ENTRY = "ConflictResolve";
const QString AbbrowserConduitConfig::PILOT_OTHER_MAP_ENTRY = "PilotOther";
const QString AbbrowserConduitConfig::PILOT_STREET_TYPE_ENTRY = "PilotStreet";
const QString AbbrowserConduitConfig::PILOT_FAX_TYPE_ENTRY = "PilotFax";
const QString AbbrowserConduitConfig::CLOSE_ABBROWSER_ON_EXIT = "CloseAbbrowser";
const QString AbbrowserConduitConfig::FIRST_TIME_SYNCING = "FirstSync";


/* 
 *  Constructs a AbbrowserConduitConfig which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
AbbrowserConduitConfig::AbbrowserConduitConfig( KConfig &c, QWidget* parent,
						const char* name, WFlags fl )
      : AbbrowserConduitConfig_base( parent, name, fl )
    {
    fConflictResolution->insertItem("User Choose",
				    (int) AbbrowserConduit::eUserChoose);
    fConflictResolution->insertItem("Keep Both in Abbrowser",
				 (int) AbbrowserConduit::eKeepBothInAbbrowser);
    fConflictResolution->insertItem("Pilot Overrides",
				    (int) AbbrowserConduit::ePilotOverides);
    fConflictResolution->insertItem("Abbrowser Overrides",
				    (int)AbbrowserConduit::eAbbrowserOverides);
    fConflictResolution->insertItem("Do nothing",
				    (int)AbbrowserConduit::eDoNotResolve);

    fSmartMerge->setChecked(c.readBoolEntry(SMART_MERGE_ENTRY,true));
    fConflictResolution->setCurrentItem(c.readNumEntry(CONFLICT_RESOLV_ENTRY,
				        (int)AbbrowserConduit::eUserChoose));
    fCloseAbbrowserOnExit->setChecked(c.readBoolEntry(CLOSE_ABBROWSER_ON_EXIT,
						      false));
    fFirstTimeSyncing->setChecked(c.readBoolEntry(FIRST_TIME_SYNCING, true));
    }

/*  
 *  Destroys the object and frees any allocated resources
 */
AbbrowserConduitConfig::~AbbrowserConduitConfig()
{
    // no need to delete child widgets, Qt does it all for us
}

int AbbrowserConduitConfig::commitChanges(KConfig& c)
    {
    c.writeEntry(SMART_MERGE_ENTRY,
		 (bool)fSmartMerge->isChecked());
    c.writeEntry(CONFLICT_RESOLV_ENTRY,
		 (int)fConflictResolution->currentItem());
    c.writeEntry(PILOT_OTHER_MAP_ENTRY, fMapPilotOther->currentText());
    c.writeEntry(PILOT_STREET_TYPE_ENTRY, fPilotStreetType->currentText());
    c.writeEntry(PILOT_FAX_TYPE_ENTRY, fPilotFaxType->currentText());
    c.writeEntry(CLOSE_ABBROWSER_ON_EXIT, fCloseAbbrowserOnExit->isChecked());
    c.writeEntry(FIRST_TIME_SYNCING, fFirstTimeSyncing->isChecked());
    return 0;
    }
#include "abbrowserConduitConfig.moc"
