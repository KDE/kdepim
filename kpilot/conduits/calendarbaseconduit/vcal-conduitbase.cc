/* vcal-conduitbase.cc                      KPilot
**
** Copyright (C) 2002-3 by Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** Contributions:
**    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>
**
** This file defines the vcal-conduit plugin.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

static const char *vcalconduitbase_id = "$Id$";

#include <options.h>

#include <qtimer.h>
#include <qfile.h>

#include <kmessagebox.h>
#include <kio/netaccess.h>

#include "libkcal/calendar.h"
#include "libkcal/calendarlocal.h"
#include "libkcal/calendarresources.h"
#include <kstandarddirs.h>

#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"
#include "pilotDateEntry.h"

#include "vcal-conduitbase.moc"
#include "vcalconduitSettings.h"




/******************************************************************
 *   Helper class VCalConduitBase::VCalEntry
 ******************************************************************/
VCalConduitBase::VCalEntry::VCalEntry( KCal::Incidence *inc )
	: RecordConduit::PCEntry(), mIncidence( inc ) {}
VCalConduitBase::VCalEntry::~VCalEntry() {}

QString VCalConduitBase::VCalEntry::uid() const { 
	return (mIncidence) ? (mIncidence->uid()) : (QString::null);
}
recordid_t VCalConduitBase::VCalEntry::recid() const {
	return (mIncidence) ? (mIncidence->pilotId()) : 0;
}
void VCalConduitBase::VCalEntry::setRecid( recordid_t recid ) {
	if (mIncidence) mIncidence->setPilotId( recid );
}
bool VCalConduitBase::VCalEntry::isEmpty() const {
	return ( !mIncidence );
}
bool VCalConduitBase::VCalEntry::isArchived() const {
	return ( mIncidence && ( mIncidence->syncStatus() == KCal::Incidence::SYNCDEL ) );
}
bool VCalConduitBase::VCalEntry::makeArchived(){
	FUNCTIONSETUP;
	if ( mIncidence ) mIncidence->setSyncStatus( KCal::Incidence::SYNCDEL );
	return true;
}
bool VCalConduitBase::VCalEntry::insertCategory( QString newcat ) {
	FUNCTIONSETUP;
	if ( !mIncidence ) return false;
		
	QStringList cats = mIncidence->categories();
	if ( !cats.contains( newcat ) ) {
		cats.append( newcat );
		mIncidence->setCategories( cats );
	}
	return true;
}


/******************************************************************
 *   Helper class VCalConduitBase::VCalDataBase
 ******************************************************************/
VCalConduitBase::VCalDataBase::VCalDataBase( RecordConduit *conduit, VCalConduitSettings*cfg ) 
   : RecordConduit::PCData( conduit ), mCalendar( 0 ), mIncidences(), mConfig( cfg )
{
	mIncidences.setAutoDelete(false);
}
VCalConduitBase::VCalDataBase::~VCalDataBase() {}

		/** Load the data from the PC ( e.g. contacts from the addressbook ).
		 *  @return true if successful, false if not */
bool VCalConduitBase::VCalDataBase::loadData() 
{
	FUNCTIONSETUP;
	
	KConfig korgcfg( locate( "config", CSL1("korganizerrc") ) );
	// this part taken from adcalendarbase.cpp:
	korgcfg.setGroup( "Time & Date" );
	QString tz( korgcfg.readEntry( "TimeZoneId" ) );
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": KOrganizer's time zone = " << tz << endl;
#endif

	// Need a subclass ptr. for the ResourceCalendar methods
	KCal::CalendarResources *rescal = 0L;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Got calendar type " << config()->calendarType() << endl;
#endif

	switch ( config()->calendarType() )
	{
		case VCalConduitSettings::eCalendarLocal:
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname
				 << "Using CalendarLocal, file="
				 << config()->calendarFile() << endl;
#endif
			if ( config()->calendarFile().isEmpty() )
			{
#ifdef DEBUG
				DEBUGCONDUIT << fname
					 << "Empty calendar file name." << endl;
#endif
				if ( mConduit ) 
					mConduit->addLogError( i18n( "You selected to sync with the a iCalendar file, "
						"but did not give a filename. Please select a valid file name in "
						"the conduit's configuration dialog" ) );
				return false;
			}

			mCalendar = new KCal::CalendarLocal( tz );
			if ( !mCalendar )
			{
				kdWarning() << k_funcinfo
					 << "Cannot initialize calendar object for file "
					 << config()->calendarFile() << endl;
				return false;
			}
#ifdef DEBUG
			DEBUGCONDUIT << fname << "Calendar's timezone: "
				 << mCalendar->timeZoneId() << endl;
			DEBUGCONDUIT << fname << "Calendar is local time: "
				 << mCalendar->isLocalTime() << endl;
#endif
			if ( mConduit )
				mConduit->addLogMessage(i18n("Using %1 time zone: %2")
					.arg(mCalendar->isLocalTime() ?
						i18n("non-local") : i18n("local"))
					.arg(tz));

			KURL kurl( config()->calendarFile() );
			if( !KIO::NetAccess::download( config()->calendarFile(), mCalendarFile, 0L ) &&
				!kurl.isLocalFile() )
			{
				if ( mConduit ) 
					mConduit->addLogError( i18n( "You chose to sync with the file \"%1\", which "
							"cannot be opened. Please make sure to supply a "
							"valid file name in the conduit's configuration dialog. "
							"Aborting the conduit." ).arg( config()->calendarFile() ) );
				KIO::NetAccess::removeTempFile( mCalendarFile );
				return false;
			}

			// if there is no calendar yet, use a first sync..
			// the calendar is initialized, so nothing more to do...
			if ( !dynamic_cast<KCal::CalendarLocal*>(mCalendar)->load( mCalendarFile ) )
			{
#ifdef DEBUG
				DEBUGCONDUIT << fname
					 << "Calendar file "
					 << mCalendarFile
					 << " could not be opened. "
					   "Will create a new one"
					 << endl;
#endif
				// Try to create empty file. if it fails,
				// no valid file name was given.
				QFile fl( mCalendarFile );
				if ( !fl.open( IO_WriteOnly | IO_Append ) )
				{
#ifdef DEBUG
					DEBUGCONDUIT << fname
						 << "Invalid calendar file name "
						 << mCalendarFile << endl;
#endif
					if ( mConduit )
						mConduit->addLogError( i18n( "You chose to sync with the file \"%1\", which "
							"cannot be opened or created. Please make sure to supply a "
							"valid file name in the conduit's configuration dialog. "
							"Aborting the conduit.").arg( config()->calendarFile() ) );
					return false;
				}
				fl.close();
//				fFirstSync = true;
			}
			if ( mConduit )
				mConduit->addSyncLogEntry( i18n( "Syncing with file \"%1\"" ).arg( config()->calendarFile() ) );
			break;
		}

		case VCalConduitSettings::eCalendarResource:
#ifdef DEBUG
			DEBUGCONDUIT << "Using CalendarResource!" << endl;
#endif
			rescal = new KCal::CalendarResources( tz );
			mCalendar = rescal;
			if ( !mCalendar )
			{
				kdWarning() << k_funcinfo << "Cannot initialize calendar " << 
					"object for ResourceCalendar" << endl;
				return false;
			}
#if LIBKCAL_IS_VERSION(1,1,0)
			rescal->readConfig();
			rescal->load();
#endif
			if ( mConduit ) {
				mConduit->addSyncLogEntry( i18n( "Syncing with standard calendar resource.") );
				mConduit->addLogMessage( i18n( "Using %1 time zone: %2" )
					.arg( mCalendar->isLocalTime() ?
						i18n("non-local") : i18n("local") )
					.arg( tz ) );
			}
			break;
		default:
			break;
	}

	if ( !mCalendar )
	{
		kdWarning() << k_funcinfo << "Unable to initialize calendar object. Please check the conduit's setup." << endl;
		if ( mConduit )
			mConduit->addLogError( i18n( "Unable to initialize the calendar object. Please check the conduit's setup" ) );
		return false;
	}
	setChanged( false );
/* @TODO: Lock the calendar!
*/
	return ( mCalendar != 0 );
}
	

		/** Save the PC data ( e.g. contacts to the addressbook ).
		 *  @return true if successful, false if not */
bool VCalConduitBase::VCalDataBase::saveData()
{
	FUNCTIONSETUP;

	if ( !mCalendar ) return false;
	
	bool res = changed();
	if ( changed() ) {
		switch( config()->calendarType() )
		{
			case VCalConduitSettings::eCalendarLocal: {
				KURL kurl( config()->calendarFile() );
				dynamic_cast<KCal::CalendarLocal*>(mCalendar)->save( mCalendarFile );
				if( !kurl.isLocalFile() )
				{
					if ( !KIO::NetAccess::upload( mCalendarFile, config()->calendarFile(), 0L ) ) 
					{
						if ( mConduit )
							mConduit->addLogError( i18n( "An error occurred while uploading \"%1\". You can try to upload "
							"the temporary local file \"%2\" manually." )
							.arg( config()->calendarFile() ).arg( mCalendarFile ) );
					}
					else {
						KIO::NetAccess::removeTempFile( mCalendarFile );
						res = true;
					}
				}
				break; }
			case VCalConduitSettings::eCalendarResource:
				mCalendar->save();
				res = true;
				break;
			default:
				break;
		}
	}
	mCalendar->close();
	KPILOT_DELETE(mCalendar);
	return res;
}	
	

		/** Return true if the data on the pc ( e.g. addressbook, calendar etc. ) is empty
		*/
bool VCalConduitBase::VCalDataBase::isEmpty() const
{
  return ( !mCalendar ) || ( mIncidences.size() < 1 );
}
		/** reset the data pointer to the beginning of the data, e.g. reset an 
		 *  iterator to begin()
		*/
RecordConduit::PCData::Iterator VCalConduitBase::VCalDataBase::begin()
{
	return Iterator( mIncidences );
}
		/** Return true if the pc data is at the end of its list.
		*/
bool VCalConduitBase::VCalDataBase::atEnd( const PCData::Iterator &it )
{
	const Iterator *it1 = dynamic_cast<const Iterator*>( it.self() );
	return ( mCalendar && it1 ) ? ( mIncidences.end() == it1->mIt ) : ( true );
}
		/** Return next modified entry in the data. 
		*/
bool VCalConduitBase::VCalDataBase::increaseNextModified( PCData::Iterator &it )
{
	++it;
	while ( !atEnd( it ) ) {
		PCEntry *pc = *it;
		VCalEntry *vc = dynamic_cast<VCalEntry*>( pc );
		KCal::Incidence *tmp = 0;
		if ( vc ) tmp = vc->incidence();
		KPILOT_DELETE( pc );
		if ( tmp && ( tmp->syncStatus()==KCal::Incidence::SYNCMOD ) ) 
			return true;
		++it;
	}
	return false;
}

RecordConduit::PCEntry *VCalConduitBase::VCalDataBase::findByUid( QString uid ) const
{
	KCal::Incidence*inc = mCalendar->incidence( uid );
	return new VCalEntry( inc );
}
const QStringList VCalConduitBase::VCalDataBase::uids() const 
{
	QStringList ids;
	for ( KCal::Incidence::List::ConstIterator it = mIncidences.begin(); it !=  mIncidences.end(); ++it )
	{
		ids.append( ( *it )->uid() );
	}
	return ids;
}
		/** Update the entry given. If it doesn't exist yet, add it
		 */
bool VCalConduitBase::VCalDataBase::updateEntry( const RecordConduit::PCEntry* entry ) 
{
	// Add to calendar if not already there
	const VCalEntry *vcalEntry = dynamic_cast<const VCalEntry*>(entry);
	if ( !vcalEntry ) return false;
	KCal::Incidence *newInc( vcalEntry->incidence() );
	KCal::Incidence *oldInc( mCalendar->incidence( newInc->uid() ) );
	if ( newInc != oldInc ) {
		mCalendar->addIncidence( newInc );
		if ( oldInc ) mCalendar->deleteIncidence( oldInc );
	}
	return true;
}
bool VCalConduitBase::VCalDataBase::removeEntry( const RecordConduit::PCEntry* entry ) 
{
	const VCalEntry *vcalEntry = dynamic_cast<const VCalEntry *>( entry );
	if ( vcalEntry && vcalEntry->incidence() && mCalendar ) {
		mIncidences.remove( vcalEntry->incidence() );
		return mCalendar->deleteIncidence( vcalEntry->incidence() );
	}
	return false;
}





/****************************************************************************
 *                          VCalConduitBase class                               *
 ****************************************************************************/


VCalConduitBase::VCalConduitBase( QString name, KPilotDeviceLink *d,
	const char *n, const QStringList &a) :
	RecordConduit( name, d, n, a )
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << vcalconduitbase_id << endl;
#endif
}



VCalConduitBase::~VCalConduitBase()
{
//	FUNCTIONSETUP;
}

/* virtual */ void VCalConduitBase::readConfig()
{
	config()->readConfig();
	SyncAction::ConflictResolution res = 
		(SyncAction::ConflictResolution)( config()->conflictResolution() );
	setConflictResolution( res );
	setArchiveDeleted( config()->syncArchived() );
}


void VCalConduitBase::setCategory( KCal::Incidence */*toIncidence*/, const PilotAppCategory */*fromRecord*/ )
{
	// @TODO
}
void VCalConduitBase::setCategory( PilotAppCategory */*toRecord*/, const KCal::Incidence */*fromIncidence*/ )
{
	// @TODO
}




/*
// return how to resolve conflicts. for now PalmOverrides=0=false, PCOverrides=1=true, Ask=2-> ask the user using a messagebox
int VCalConduitBase::resolveConflict(KCal::Incidence*e, PilotAppCategory*de) {
	if (getConflictResolution()==SyncAction::eAskUser)
	{
		// TODO: This is messed up!!!
		return KMessageBox::warningYesNo( 0,
			i18n("The following item was modified both on the Pilot and on your PC:\nPC entry:\n\t")+e->summary()+i18n("\nPilot entry:\n\t")+getTitle(de)+
				i18n("\n\nShould the Pilot entry overwrite the PC entry? If you select \"No\", the PC entry will overwrite the Pilot entry."),
			i18n("Conflicting Entries")
		)==KMessageBox::No;
	}
	return getConflictResolution();
}

*/

bool VCalConduitBase::smartMergeEntry( RecordConduit::PCEntry *pcEntry, PilotAppCategory *backupEntry,
		PilotAppCategory *palmEntry )
{
	int confRes = getConflictResolution();
	if ( confRes == SyncAction::eAskUser )
	{
		const VCalEntry *vcalEntry = dynamic_cast<const VCalEntry *>( pcEntry );
		if ( vcalEntry && vcalEntry->incidence() && KMessageBox::warningYesNo(NULL,
			i18n("The following item was modified both on the Pilot and on your PC:\n"
				"PC entry:\n"
				"\t%1\n"
				"Handheld entry:\n"
				"\t%1\n\n"
				"Shall the entry from the handheld overwrite the PC entry? If you select "
				"\"No\", the PC entry will overwrite the Pilot entry.")
				.arg( vcalEntry->incidence()->summary() )
				.arg( getTitle( palmEntry ) ),
			i18n("Conflicting Entries")
		)==KMessageBox::No ) {
			confRes = SyncAction::eCopyPCToHH;
		} else {
			confRes = SyncAction::eCopyHHToPC;
		}
	}
	bool result = true;
	switch ( confRes ) {
		case eHHOverrides:
			return result && palmCopyToPC( pcEntry, backupEntry, palmEntry );
		case ePCOverrides:
			return result && pcCopyToPalm( pcEntry, backupEntry, palmEntry );
		case ePreviousSyncOverrides:
			_copy( pcEntry, backupEntry );
			if ( palmEntry && backupEntry ) *palmEntry = *backupEntry;
			result &=  palmSaveEntry( backupEntry, pcEntry );
			result &=  pcSaveEntry( pcEntry, backupEntry, backupEntry );
			return result;
		case eDuplicate:
			// Set the Palm ID to 0 so we don't overwrite the existing record.
			pcEntry->setRecid( 0 );
			result &= pcCopyToPalm( pcEntry, 0L, 0L );
			{
			// @TODO: This should creat
			VCalEntry pcent( 0 );
			result &=  palmCopyToPC( &pcent, backupEntry, palmEntry );
			}
			return result;
		case eDelete:
			return result && pcDeleteEntry( pcEntry, backupEntry, palmEntry );
		case eAskUser:
		case eDoNothing:
			break;
	}
}
