/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <time.h>

#include <qlabel.h>
#include <qlistview.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>

#include <qtextedit.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstatusbar.h>

#include <kabc/stdaddressbook.h>

#include "modem.h"
#include "atcommand.h"
#include "commandscheduler.h"

#include "mobilegui.h"
#include "mobilegui.moc"
#include "mobilemain.h"


class SyncEntry
{
  public:
    SyncEntry()
    {
      mOn = true;
      mToBeUpdated = false;
      mToBeInserted = false;
    }
  
    bool mOn;
    bool mToBeUpdated;
    bool mToBeInserted;
};


class SyncEntryKab : public SyncEntry
{
  public:
    SyncEntryKab( bool on, const QString &index, const QString &name,
                  const QString &phone )
    {
      mOn = on;
      
      mIndex = index;
      mName = name;
      mPhone = phone;
      
      mKABindex = -1;
      mPhoneNumberIndex = -1;
    }
  
    QString mIndex;
    QString mName;
    QString mPhone;

    KABC::Addressee mAddressee;
    int mKABindex;
    int mPhoneNumberIndex;
};


class SyncEntryMobile : public SyncEntry
{
  public:
    SyncEntryMobile( bool on, const QString &index, const QString &phone,
                     const QString &name )
    {
      mOn = on;
      mToBeDeleted = false;

      mIndex = index;
      mName = name;
      mPhone = phone;
    }
    
    QString mIndex;
    QString mName;
    QString mPhone;
    
    bool mToBeDeleted;
};


class SyncEntryCommon : public SyncEntry
{
  public:
    SyncEntryCommon( bool on, SyncEntryKab *kabEntry,
                     SyncEntryMobile *mobileEntry )
    {
      mOn = on;
      mKabEntry = kabEntry;
      mMobileEntry = mobileEntry;
    }
    
    SyncEntryKab *mKabEntry;
    SyncEntryMobile *mMobileEntry;
};


class AddressSyncer
{
  public:
    AddressSyncer()
    {
      mKabEntries.setAutoDelete( true );
      mMobileEntries.setAutoDelete( true );
      mCommonEntries.setAutoDelete( true );
    }
  
    QPtrList<SyncEntryKab> mKabEntries;
    QPtrList<SyncEntryMobile> mMobileEntries;
    QPtrList<SyncEntryCommon> mCommonEntries; 
};


class PhoneBookItem : public QCheckListItem
{
  public:
    PhoneBookItem( QListView *v ) :
      QCheckListItem( v, "", QCheckListItem::CheckBox )
    {
      mSyncEntry = 0;
    }

    PhoneBookItem( QListView *v, SyncEntry *syncEntry, const QString &name,
                   const QString &phone, const QString &index ) :
      QCheckListItem( v, index, QCheckListItem::CheckBox )
    {
      mSyncEntry = syncEntry;
      
      setText( 0, name );
      setText( 1, phone );
      setText( 2, index );
    }

    SyncEntry *syncEntry() { return mSyncEntry; }

  private:
    SyncEntry *mSyncEntry;
};


/* 
 *  Constructs a MobileGui which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
MobileGui::MobileGui( CommandScheduler *scheduler, KandyPrefs *kprefs,
                      QWidget* parent, const char* name, WFlags fl ) :
  DCOPObject( "KandyIface" ), MobileGui_base( parent, name, fl )
{
  // Setup links to related classes
  mScheduler = scheduler;
  mSyncer = new AddressSyncer;
  mPrefs = kprefs;
  mparent = parent;

  // Setup mobile phone specific data
  mMobManufacturer = "";
  mMobModel = "";
  mPBStartIndex = 0;
  mPBLength = 0;
  mPBNameLength = 0;
  mPBIndexOccupied.resize( 0, false );
  mMobHasFD = false;
  mMobHasLD = false;
  mMobHasME = false;
  mMobHasMT = false;
  mMobHasTA = false;
  mMobHasOW = false;
  mMobHasMC = false;
  mMobHasRC = false;
  
  // Setup status for asynchronous control flow
  mLastWriteId = "";
  mComingFromToggleConnection = false;
  mComingFromReadPhonebook = false;
  mComingFromSyncPhonebooks = false;
  mComingFromExit = false;
  
  // Setup initial state of phone books
  setKabState( UNLOADED );
  setMobState( UNLOADED );

  // Setup signal handlers
  connect( mScheduler, SIGNAL( commandProcessed( ATCommand * ) ),
           SLOT( processResult( ATCommand * ) ) );
  connect( mScheduler->modem(), SIGNAL( gotLine( const char * ) ),
           SLOT( termAddOutput( const char * ) ) );
}


MobileGui::~MobileGui()
{
  delete mSyncer;
}


void MobileGui::exit()
{
  warnKabState( UNLOADED );
  
  mComingFromExit = true;
  if ( !warnMobState( UNLOADED ) ) {
    mComingFromExit = false;
    kapp->quit();
  }
}


void MobileGui::readModelInformation()
{
  // Read generic manufacturer and model information
  mScheduler->executeId( "+cgmi" );
  mScheduler->executeId( "+cgmm" );
  mScheduler->executeId( "+cgmr" );
  mScheduler->executeId( "+cgsn" );
  
  // Read information about additional phonebook memories
  ATCommand *cmd = new ATCommand( "+cpbs=?" );
  cmd->setAutoDelete( true );
  mScheduler->execute( cmd );
  
  // Select SIM phonebook by default
  cmd = new ATCommand( "+cpbs=" );
  cmd->setAutoDelete( true );
  cmd->addParameter( new ATParameter( "SM" ) );
  mScheduler->execute( cmd );

  // Read phonebook properties
  mScheduler->executeId( "+cpbr=?" );
  mScheduler->executeId( "+cpbs?" );

  // Set clock
  if ( (*mPrefs).autoSetClock() )
    setClock();
}


void MobileGui::readPhonebook()
{
  if ( mMobState == LOADED )
    return;

  mComingFromReadPhonebook = true;
  if ( !warnMobState( LOADED ) ) {
    mComingFromReadPhonebook = false;
    QString tmp = "";
    
    ATCommand *cmd = new ATCommand( "+cpbr=" );
    cmd->setAutoDelete( true );
    cmd->addParameter( new ATParameter( tmp.setNum( mPBStartIndex ) ) );
    cmd->addParameter( new ATParameter( tmp.setNum( mPBStartIndex +
                                                    mPBLength - 1 ) ) );
    
    mScheduler->execute( cmd );
  
    emit statusMessage( i18n( "Reading mobile phonebook..." ) );
  }
}


void MobileGui::writePhonebook()
{
  bool ModemCommandScheduled = false;
  
  
  if ( mMobState != MODIFIED )
    return;

  PushButton12->setEnabled( false );


  //
  // Remove all entries from data structures which are marked as
  // deleted but which are not found on the mobile phone
  //
  
  for ( uint i = 0; i < mSyncer->mMobileEntries.count(); i++ ) {
    SyncEntryMobile *entry = mSyncer->mMobileEntries.at( i );
    
    
    if ( entry->mToBeDeleted )
      if ( entry->mIndex.isEmpty() ) {
        // The current entry has to be deleted but doesn't come from
	// the mobile phone. Hence, it was inserted during phonebook
	// synchronisation or so.
	// => It is sufficient to remove it from mMobileEntries, no
	// ATCommand for deletion needs to be scheduled.
        mSyncer->mMobileEntries.remove( i );
        i--;
      } else {
        // The current entry has to be deleted and stems from the
	// mobile phone. First thing to do is to free its associated
	// index. This way, its index can be reused for entries which
	// have be newly inserted to the mobile phone and we can save
	// an explicit ATCommand for deletion and save time & battery
	// energy.
	uint theIndex = entry->mIndex.toUInt();
	mPBIndexOccupied[ theIndex - mPBStartIndex ] = false;
      }
  }

  
  //
  // Write all elements which need an update to the mobile phone
  //
  
  for ( uint i = 0; i < mSyncer->mMobileEntries.count(); i++ ) {
    SyncEntryMobile *entry = mSyncer->mMobileEntries.at( i );
    QString id;


    // Only process changed items of the mobile phonebook in
    // order to save time.
    if ( entry->mToBeUpdated || entry->mToBeInserted ) {
      QString tmp = "";
      
      
      if ( entry->mToBeUpdated ) {
        id = "+cpbw=" + entry->mIndex;
      } else {
        int index = firstFreeIndex();
	
	
	mPBIndexOccupied[ index ] = true;
        id = "+cpbw=" + tmp.setNum( index + mPBStartIndex );
      }
      mLastWriteId = id;
      entry->mToBeUpdated = false;
      entry->mToBeInserted = false;

      ATCommand *cmd = new ATCommand( id );
      cmd->setAutoDelete( true );
      cmd->addParameter( new ATParameter( quote( entry->mPhone ) ) );

      if ( entry->mPhone.left( 1 ) == "+" )
        cmd->addParameter( new ATParameter( "145" ) );
      else
        cmd->addParameter( new ATParameter( "129" ) );

      cmd->addParameter( new ATParameter(
                               quote( string2GSM( entry->mName ) ) ) );
    
      mScheduler->execute( cmd );
      ModemCommandScheduled = true;
    }
  }
  
  
  //
  // As a final step, we need to check again all entries which should be
  // deleted. If entries exist stemming from the mobile phone and whose
  // index-position was not reused for updating or inserting other entries in
  // the previous loop, we need to issue an explicit ATCommand for its deletion.
  //
  
  for ( uint i = 0; i < mSyncer->mMobileEntries.count(); i++ ) {
    SyncEntryMobile *entry = mSyncer->mMobileEntries.at( i );
    
    
    if ( entry->mToBeDeleted ) {
      uint theIndex = entry->mIndex.toUInt();
      
      
      if ( !mPBIndexOccupied[ theIndex - mPBStartIndex ] ) {
        // Index of item to be deleted still is 0, so that index position
	// wasn't reused. We must delete it explicitly.
	QString id = "+cpbw=" + entry->mIndex;
	
	
	mLastWriteId = id;
	ATCommand *cmd = new ATCommand( id );
	cmd->setAutoDelete( true );
	
	mScheduler->execute( cmd );
	ModemCommandScheduled = true;
      }
    
      // Remove entry from internal data structures
      mSyncer->mMobileEntries.remove( i );
      i--;
    }
  }

  if ( ModemCommandScheduled )
    emit statusMessage( i18n( "Writing mobile phonebook..." ) );
  else
    writePhonebookPostProcessing();
}


void MobileGui::writePhonebookPostProcessing()
{
  mLastWriteId = "";
  emit transientStatusMessage( i18n( "Wrote mobile phonebook." ) );
  PushButton12->setEnabled( true );
  setMobState( LOADED );
  updateMobileBook();

  if ( mComingFromToggleConnection ) {
    mComingFromToggleConnection = false;
    disconnectGUI();
  } else
  if ( mComingFromReadPhonebook ) {
    mComingFromReadPhonebook = false;
    QString tmp = "";
    
    ATCommand *cmd = new ATCommand( "+cpbr=" );
    cmd->setAutoDelete( true );
    cmd->addParameter( new ATParameter( tmp.setNum( mPBStartIndex ) ) );
    cmd->addParameter( new ATParameter( tmp.setNum( mPBStartIndex +
                                                    mPBLength - 1 ) ) );
    
    mScheduler->execute( cmd );

    emit statusMessage( i18n( "Reading mobile phonebook..." ) );
  } else
  if ( mComingFromExit ) {
    mComingFromExit = false;
    kapp->quit();
  }
}


void MobileGui::setClock()
{
  char *timeStr = (char *) malloc( 50 * sizeof( char ) );
  QString id = "+cclk=";
  ATCommand *cmd = new ATCommand( id );


  cmd->setAutoDelete( true );
  
  time_t tloc;
  time( &tloc );
  struct tm *theTime = localtime( &tloc );
  strftime( timeStr, 50, "%y/%m/%d,%T+00", theTime );

  QString Time = timeStr;
  cmd->addParameter( new ATParameter( quote( Time ) ) );
  
  mScheduler->execute( cmd );
  
  delete timeStr;
  delete theTime;
}


void MobileGui::readKabc()
{
  if ( mKabState == LOADED )
    return;

  warnKabState( LOADED );
   
  emit statusMessage( i18n( "Reading KDE address book..." ) );

  mSyncer->mKabEntries.clear();

  KABC::AddressBook *addressBook = KABC::StdAddressBook::self( true );
  KABC::AddressBook::Iterator it;
  int kabIndex = 0;

  for ( it = addressBook->begin(); it != addressBook->end();
        it++, kabIndex++ ) {
    QString index, name;
    KABC::PhoneNumber phoneNumber;
    KABC::PhoneNumber::List phoneNumbers = (*it).phoneNumbers();
    KABC::PhoneNumber::List::Iterator it2;
    int phoneNumberIndex = 0;


    // Scan all numbers associated with a KAB entry
    for ( it2 = phoneNumbers.begin(); it2 != phoneNumbers.end();
          it2++, phoneNumberIndex++ ) {
      bool excludeNumber = false;
      phoneNumber = (*it2);
      QString phone = phoneNumber.number();

	
      if ( (*mPrefs).excludeHome() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Home ) )
	excludeNumber = true;
      if ( (*mPrefs).excludeWork() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Work ) )
	excludeNumber = true;
      if ( (*mPrefs).excludeMessaging() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Msg ) )
	excludeNumber = true;
      if ( (*mPrefs).excludeFax() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Fax ) )
	excludeNumber = true;
      if ( (*mPrefs).excludeCell() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Cell ) )
	excludeNumber = true;
      if ( (*mPrefs).excludeVideo() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Video ) )
	excludeNumber = true;
      if ( (*mPrefs).excludeMailbox() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Bbs ) )
	excludeNumber = true;
      if ( (*mPrefs).excludeModem() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Modem ) )
	excludeNumber = true;
      if ( (*mPrefs).excludeCar() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Car ) )
	excludeNumber = true;
      if ( (*mPrefs).excludeISDN() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Isdn ) )
	excludeNumber = true;
      if ( (*mPrefs).excludePager() &&
           ( phoneNumber.type() & KABC::PhoneNumber::Pager ) )
	excludeNumber = true;
	
      if ( excludeNumber == false ) {
	SyncEntryKab *kabEntry;
	  
	  
	index = "";
        name = (*it).familyName();
	    
	KABC::AddressBook::Iterator it3;
	KABC::Addressee::List tmp;
	bool firstCharIsUnique = true;
	for ( it3 = addressBook->begin(); it3 != addressBook->end(); ++it3 )
	  if ( ( (*it3).familyName() == name ) && ( it3 != it ) ) {
	    tmp.append( (*it3) );
	    if ( (*it3).givenName()[0] == (*it).givenName()[0] )
	      firstCharIsUnique = false;
	  }

	// There are several KAB entries with the same family name.
	// So, we need to append the given name in order to
	// distinguish them.
	if ( ( tmp.size() > 0 ) && !(*it).givenName().isEmpty() ) {
	  name += ", ";
	      
	  if ( firstCharIsUnique )
	    name += (*it).givenName()[0] + ".";
	  else
	    name += (*it).givenName();
	}
	      
	// Truncate name field if it's too long for mobile phone
	if ( name.length() > mPBNameLength )
	  name = name.remove( mPBNameLength, name.length() - mPBNameLength );
	    
	// Append Suffix to name if specified in preferences
   	if ( (*mPrefs).useHomeSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Home ) )
	  formatPBName( &name, (*mPrefs).homeSuff() );
	else
	if ( (*mPrefs).useWorkSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Work ) )
	  formatPBName( &name, (*mPrefs).workSuff() );
	else
	if ( (*mPrefs).useMessagingSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Msg ) )
	  formatPBName( &name, (*mPrefs).messagingSuff() );
	else
	if ( (*mPrefs).useFaxSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Fax ) )
	  formatPBName( &name, (*mPrefs).faxSuff() );
	else
	if ( (*mPrefs).useCellSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Cell ) )
	  formatPBName( &name, (*mPrefs).cellSuff() );
	else
	if ( (*mPrefs).useVideoSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Video ) )
	  formatPBName( &name, (*mPrefs).videoSuff() );
	else
	if ( (*mPrefs).useMailboxSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Bbs ) )
	  formatPBName( &name, (*mPrefs).mailboxSuff() );
	else
	if ( (*mPrefs).useModemSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Modem ) )
	  formatPBName( &name, (*mPrefs).modemSuff() );
	else
	if ( (*mPrefs).useCarSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Car ) )
	  formatPBName( &name, (*mPrefs).carSuff() );
	else
	if ( (*mPrefs).useISDNSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Isdn ) )
	  formatPBName( &name, (*mPrefs).iSDNSuff() );
	else
	if ( (*mPrefs).usePagerSuff() &&
	     ( phoneNumber.type() & KABC::PhoneNumber::Pager ) )
	  formatPBName( &name, (*mPrefs).pagerSuff() );
	    
	kabEntry = new SyncEntryKab( true, index, name, phone );
	kabEntry->mKABindex = kabIndex;
	kabEntry->mPhoneNumberIndex = phoneNumberIndex;
	
	kabEntry->mAddressee = (*it);
    	mSyncer->mKabEntries.append( kabEntry );
      }
    }
  }

  // Display KAB entries
  updateKabBook();
  
  emit transientStatusMessage( i18n( "Read KDE address book." ) );
  
  setKabState( LOADED );
}


QString MobileGui::decodeSuffix( const QString &suffix )
{
  QString theSuffix = suffix;
  
  
  // Check whether suffix is quoted. If so, it should be interpreted
  // as Hex-Number of a special GSM character.
  if ( ( theSuffix.left( 1 ) == "\"" ) && ( theSuffix.right( 1 ) == "\"" ) ) {
    QString tmp = "";
    char suffixNumber = (char) dequote( suffix ).toUInt( 0, 16 );
    tmp += suffixNumber;

    theSuffix = GSM2String( tmp );
  }
  
  return theSuffix;
}


void MobileGui::formatPBName( QString *name, QString suffix )
{
  QString theSuffix = decodeSuffix( suffix );

  
  if ( name->length() + theSuffix.length() > mPBNameLength ) {
    // Truncate name field if it's too long for mobile phone
    unsigned int toolong = name->length() + theSuffix.length() - mPBNameLength;
    (*name) = name->remove( name->length() - toolong, toolong );
  } else
  if ( name->length() + theSuffix.length() < mPBNameLength )
    // Add white spaces so that suffix is right justified
    while ( name->length() + theSuffix.length() != mPBNameLength )
      (*name) += ' ';

  (*name) += theSuffix;
}


QString MobileGui::stripWhiteSpaces( const QString &theString )
{
  int pos = 0;
  int len = theString.length();


  for ( unsigned int i = 0; i < theString.length(); i++ )
    if ( theString[ i ].latin1() == ' ' ) {
      pos++;
      len--;
    } else
      break;

  if ( len == 0 )
    return "";

  for ( int i = theString.length() - 1; i >= 0; i-- )
    if ( theString[ i ].latin1() == ' ' )
      len--;
    else
      break;
  
  return theString.mid( pos, len );
}


void MobileGui::writeKabc()
{
  if ( mKabState != MODIFIED )
    return;

  KABC::AddressBook *addressBook = KABC::StdAddressBook::self( true );
  KABC::Ticket *ticket = addressBook->requestSaveTicket();

  if ( !ticket ) {
    kdDebug() << "Error! No ticket to save." << endl;
    return;
  }


  for ( uint i = 0; i < mSyncer->mKabEntries.count(); i++ ) {
    SyncEntryKab *kabEntry = mSyncer->mKabEntries.at( i );
    QString phoneNumber = kabEntry->mPhone;
    
    
    if ( kabEntry->mToBeUpdated ) {
      // Find the entry in the KAB which has to be updated
      KABC::AddressBook::Iterator it = addressBook->begin();
      for ( int KABindex = 0; KABindex != kabEntry->mKABindex;
            it++, KABindex++ ) ;
	
      // Find the correct phonenumber of the phonebook entry
      KABC::PhoneNumber::List phoneNumbers = (*it).phoneNumbers();
      KABC::PhoneNumber::List::Iterator it2 = phoneNumbers.begin();
      for ( int phoneNumberIndex = 0;
            phoneNumberIndex != kabEntry->mPhoneNumberIndex;
	    it2++, phoneNumberIndex++ ) ;
      
      (*it2).setNumber( phoneNumber ); 
      (*it).insertPhoneNumber( (*it2) );
    } else

    if ( kabEntry->mToBeInserted ) {
      int phoneType = 0;
      bool goon = true;
      KABC::AddressBook::Iterator it;
      bool equivalentEntryFound = false;
      QString name = kabEntry->mName;


      //
      // Identify Type of Phonenumber using possibly appended suffixes.
      // If a suffix is found, remove it from the name.
      //
      if ( goon && (*mPrefs).useHomeSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).homeSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Home;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }
      if ( goon && (*mPrefs).useWorkSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).workSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Work;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }
      if ( goon && (*mPrefs).useMessagingSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).messagingSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Msg;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }
      if ( goon && (*mPrefs).useFaxSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).faxSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Fax;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }
      if ( goon && (*mPrefs).useCellSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).cellSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Cell;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }
      if ( goon && (*mPrefs).useVideoSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).videoSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Video;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }
      if ( goon && (*mPrefs).useMailboxSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).mailboxSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Bbs;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }
      if ( goon && (*mPrefs).useModemSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).modemSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Modem;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }
      if ( goon && (*mPrefs).useCarSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).carSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Car;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }
      if ( goon && (*mPrefs).useISDNSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).iSDNSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Isdn;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }
      if ( goon && (*mPrefs).usePagerSuff() ) {
        QString theSuffix = decodeSuffix( (*mPrefs).pagerSuff() );
	if ( name.right( theSuffix.length() ) == theSuffix ) {
	  phoneType = KABC::PhoneNumber::Pager;
	  name = stripWhiteSpaces(
	           name.left( name.length() - theSuffix.length() ) );
	  goon = false;
	}
      }


      //
      // Search for a KAB entry whose name, if formatted in exactly the
      // same way as was done in readKabc, is equal to the actual name.
      //
      
      for ( it = addressBook->begin(); it != addressBook->end(); it++ ) {
        QString kabName = (*it).familyName();
	KABC::AddressBook::Iterator it3;
	KABC::Addressee::List tmp;
	bool firstCharIsUnique = true;
	unsigned int minLength;


	for ( it3 = addressBook->begin(); it3 != addressBook->end(); it3++ )
	  if ( ( (*it3).familyName() == kabName ) && ( it3 != it ) ) {
	    tmp.append( (*it3) );
	    if ( (*it3).givenName()[0] == (*it).givenName()[0] )
	      firstCharIsUnique = false;
	  }

	// There are several KAB entries with the same family name.
	// So, we need to append the given name in order to
	// distinguish them.
	if ( ( tmp.size() > 0 ) && !(*it).givenName().isEmpty() ) {
	  kabName += ", ";
	      
	  if ( firstCharIsUnique )
	    kabName += (*it).givenName()[0] + ".";
	  else
	    kabName += (*it).givenName();
	}
	      
	// Truncate name field if it's too long for mobile phone
	if ( kabName.length() > mPBNameLength )
	  kabName = kabName.remove( mPBNameLength,
	                            kabName.length() - mPBNameLength );

        minLength = kabName.length();
	if ( name.length() < minLength )
	  minLength = name.length();
	  
	if ( name.left( minLength ) == kabName.left( minLength ) ) {
	  (*it).insertPhoneNumber( KABC::PhoneNumber( phoneNumber,
	                                              phoneType ) );
	  
	  equivalentEntryFound = true;
	  break;
	}
      }
      
      //
      // If no equivalent entry was found in KAB, we need to generate
      // a complete new entry.
      //
      
      if ( !equivalentEntryFound ) {
        KABC::Addressee entry;
        QStringList *fields = new QStringList;

  
        *fields = QStringList::split( ',', name );
	
	if ( fields->count() > 1 ) {
	  // Name string contains comma separated entry so that we
	  // need to build family and given names out of them.
	  QString givenName = "";
	  

	  entry.setFamilyName( stripWhiteSpaces( (*fields)[ 0 ] ) );
	  
	  for ( unsigned int i = 1; i < fields->count(); i++ )
	    givenName += stripWhiteSpaces( (*fields)[ i ] ) + " ";
	  entry.setGivenName( stripWhiteSpaces( givenName ) );
	} else
	  // Name string contains only one string without comma.
	  entry.setFamilyName( stripWhiteSpaces( name ) );

        entry.insertPhoneNumber( KABC::PhoneNumber( phoneNumber, phoneType ) );

        addressBook->insertAddressee( entry );
      }
    }

    kabEntry->mToBeUpdated = false;
    kabEntry->mToBeInserted = false;
  }
  
  addressBook->save( ticket );
  
  emit transientStatusMessage( i18n( "Wrote KDE address book." ) );
  
  setKabState( LOADED );
}


void MobileGui::refreshStatus()
{
  mScheduler->executeId( "+cbc" );
  mScheduler->executeId( "+csq" );
}


void MobileGui::processResult( ATCommand *command )
{
  if ( command->id() == "+cbc" )
    mBatteryChargeLabel->setText( command->resultField( 1 ) + " %" );
  else
  if ( command->id() == "+csq" )
    mSignalQualityLabel->setText( command->resultField( 0 ) );
  else
  if ( command->id() == "+cgmi" ) {
    mMobManufacturer = command->resultField( 0 );
    mManufacturerLabel->setText( mMobManufacturer );
  } else
  if ( command->id() == "+cgmm" ) {
    mMobModel = command->resultField( 0 );
    mModelLabel->setText( mMobModel );
  } else
  if ( command->id() == "+cgmr" )
    mGSMVersionLabel->setText( command->resultField( 0 ) );
  else
  if ( command->id() == "+cgsn" )
    mSerialNumberLabel->setText( command->resultField( 0 ) );
  else
  if ( command->id() == "+cpbr=?" )
  {
    QStringList tmpList = QStringList::split( "-", command->resultField( 0 ) );
    QString tmpString = tmpList.first().right( tmpList.first().length() - 1 );
    mPBStartIndex = tmpString.toUInt();
    mPBNameLength = command->resultField( 2 ).toUInt();
  } else
  if ( command->id() == "+cpbs?" ) {
    mPBLength = command->resultField( 2 ).toUInt();

    // Allocate and initialize memory for the buckets of indices
    mPBIndexOccupied.resize( mPBLength, false );
    for ( unsigned int i = 0; i < mPBLength; i++ )
      mPBIndexOccupied[ i ] = false;
  } else
  if ( command->id() == "+cpbr=" ) {
    fillPhonebook( command );
    
    if ( mComingFromSyncPhonebooks ) {
      mComingFromSyncPhonebooks = false;
      mergePhonebooks();
    }
  } else
  if ( command->id() == mLastWriteId )
    writePhonebookPostProcessing();
  else
  if ( command->id() == "+cpbs=?" ) {
    QPtrList<QStringList> *list = command->resultFields();
    QStringList *fields = list->first();


    while( fields ) {
      for ( unsigned int i = 0; i < fields->count(); i++ ) {
        QString memory = dequote( (*fields)[ i ] );


	if ( memory == "FD" )
	  mMobHasFD = true;
        else
	if ( memory == "LD" )
	  mMobHasLD = true;
	else
	if ( memory == "ME" )
	  mMobHasME = true;
	else
	if ( memory == "MT" )
	  mMobHasMT = true;
	else
	if ( memory == "TA" )
	  mMobHasTA = true;
	else
	if ( ( memory == "OW" ) ||
	     ( ( memory == "ON" ) && ( mMobManufacturer == "SIEMENS" ) ) )
	  mMobHasOW = true;
	else
	if ( ( mMobManufacturer == "SIEMENS" ) && ( memory == "MC" ) )
	  mMobHasMC = true;
	else
	if ( ( mMobManufacturer == "SIEMENS" ) && ( memory == "RC" ) )
	  mMobHasRC = true;
      }
      
      fields = list->next();
    }
  }
}


QString MobileGui::noSpaces( const QString &theString )
{
  QString result = "";
  
  
  for ( unsigned int i = 0; i < theString.length(); i++ )
    if ( theString[ i ].latin1() != ' ' )
      result += theString[ i ];
  
  return result;
}


int MobileGui::firstFreeIndex()
{
  unsigned int i;
  
  
  if ( mPBIndexOccupied.capacity() == 0 )
    return 0;

  for ( i = 1; i < mPBLength; i++ )
    if ( !mPBIndexOccupied[ i ] )
      break;

  if ( i < mPBLength )
    return i;

  return 0;
}


QString MobileGui::string2GSM( const QString &theString )
{
  QString result = "";


  for ( unsigned int i = 0; i < theString.length(); i++ )
    switch ( theString[ i ].latin1() ) {
      case 'Ä': result += '['; break;
      case 'ä': result += '{'; break;
      case 'Ö': result += 92; break;
      case 'ö': result += '|'; break;
      case 'Ü': result += '^'; break;
      case 'ü': result += '~'; break;
      case 'ß': result += 30; break;
      case 'è': result += 4; break;
      case 'é': result += 5; break;
	  
      default: result += theString[ i ];
    }
  
  return result;
}


QString MobileGui::GSM2String( const QString &theString )
{
  QString result = "";


  for ( unsigned int i = 0; i < theString.length(); i++ )
    switch ( theString[ i ].latin1() ) {
      case '[': result += 'Ä'; break;
      case '{': result += 'ä'; break;
      case 92:  result += 'Ö'; break;
      case '|': result += 'ö'; break;
      case '^': result += 'Ü'; break;
      case '~': result += 'ü'; break;
      case 30:  result += 'ß'; break;
      case 4:   result += 'è'; break;
      case 5:   result += 'é'; break;

      default: result += theString[ i ];
    }

  return result;
}


void MobileGui::fillPhonebook( ATCommand *cmd )
{
  mSyncer->mMobileEntries.clear();
    
  QPtrList<QStringList> *list = cmd->resultFields();
  QStringList *fields = list->first();

  while( fields ) {
    if ( fields->count() != 4 )
      kdDebug() << "Error! Unexpected number of address fields." << endl;
    else {
      QString index = (*fields)[0];
      QString phone = (*fields)[1];
      QString type = (*fields)[2];
      QString name = GSM2String( (*fields)[3] );

      SyncEntryMobile *phoneEntry = new SyncEntryMobile( true, dequote( index ),
                                                         dequote( phone ),
							 dequote( name ) );
      mPBIndexOccupied[ index.toUInt() - mPBStartIndex ] = true;
      mSyncer->mMobileEntries.append( phoneEntry );
    }
    fields = list->next();
  }

  // Display mobile entries
  updateMobileBook();

  emit transientStatusMessage(i18n("Read mobile phonebook."));
  emit phonebookRead();
  
  setMobState( LOADED );
}


QString MobileGui::quote( const QString &str )
{
  if ( ( str.left(1) == "\"" ) && ( str.right(1) == "\"" ) )
    return str;
  
  return "\"" + str + "\"";
}


QString MobileGui::dequote( const QString &str )
{
  int pos = 0;
  int len = str.length();


  if ( str.left(1) == "\"" ) {
    pos = 1;
    len --;
  } 
  
  if ( str.right(1) == "\"" )
    len--;
  
  return str.mid( pos, len );
}


void MobileGui::savePhonebook()
{
  if ( mMobState == UNLOADED )
    return;
    
  QString fileName = KFileDialog::getSaveFileName( "phonebook.csv" );
  QFile outFile( fileName );

  if ( outFile.open( IO_WriteOnly ) ) {
    QTextStream t( &outFile );        // use a text stream

    for( uint i = 0; i < mSyncer->mMobileEntries.count(); i++) {
      SyncEntryMobile *e = mSyncer->mMobileEntries.at( i );
      
      
      if ( !e->mToBeDeleted )
        t << e->mIndex << "," << e->mPhone << "," << e->mName << endl;
    }

    outFile.close();
  }
}


void MobileGui::deleteMobPhonebook()
{

  //
  // Process all elements selected in the GUI
  //
  
  PhoneBookItem *item = (PhoneBookItem *) mMobileBook->firstChild();
  while ( item ) {
    if ( item->isOn() ) {
      SyncEntryMobile *mobileItem = (SyncEntryMobile *) item->syncEntry();


      // Deselect current item
      item->setOn( false );
      mobileItem->mOn = false;
      
      // Mark current item as deleted
      mobileItem->mToBeDeleted = true;
    }
    
    item = (PhoneBookItem *) item->nextSibling();
  }
      
  // Update GUI
  updateMobileBook();
  setMobState( MODIFIED );
}


void MobileGui::mergePhonebooks()
{
  uint i;
  
  
  //
  // Transfer current Selection State from GUI to mSyncer
  //

  PhoneBookItem *item = (PhoneBookItem *) mKabBook->firstChild();
  while ( item ) {
    item->syncEntry()->mOn = item->isOn();
    item = (PhoneBookItem *) item->nextSibling();
  }

  item = (PhoneBookItem *) mMobileBook->firstChild();
  while ( item ) {
    item->syncEntry()->mOn = item->isOn();
    item = (PhoneBookItem *) item->nextSibling();
  }
  
  mSyncer->mCommonEntries.clear();


  //
  // Put KDE Address Book list into Common List
  //

  for ( i = 0; i < mSyncer->mKabEntries.count(); i++ )
    if ( mSyncer->mKabEntries.at( i )->mOn ) {
      mSyncer->mCommonEntries.append(
        new SyncEntryCommon( true, mSyncer->mKabEntries.at( i ), 0 ) );
      mSyncer->mKabEntries.at( i )->mOn = false;
    }


  //
  // Put Mobile Address Book list into Common List; Merge equivalent entries
  //
  
  for ( i = 0; i < mSyncer->mMobileEntries.count(); i++ ) {
    SyncEntryMobile *mobileEntry = mSyncer->mMobileEntries.at( i );
    bool equivalentEntryFound = false;
    uint j;
    
    
    if( !mobileEntry->mToBeDeleted )
      for ( j = 0; j < mSyncer->mCommonEntries.count(); j++ ) {
        SyncEntryCommon *theCommonEntry = mSyncer->mCommonEntries.at( j );
      
      
        if ( theCommonEntry->mKabEntry &&
	     ( theCommonEntry->mKabEntry->mName == mobileEntry->mName ) ) {
          theCommonEntry->mMobileEntry = mobileEntry;
	  equivalentEntryFound = true;
	
          if ( noSpaces( theCommonEntry->mKabEntry->mPhone ) ==
	       mobileEntry->mPhone ) {
	    mobileEntry->mOn = false;
	    break;
          } else {
	    // Conflict: 2 Entries have same name but different numbers.
	    // Prompt user.
            QString text = "<qt><b>" + i18n( "Kab Entry:" ) + "</b><br>";
            text += "  " + theCommonEntry->mKabEntry->mName + " " +
	                   theCommonEntry->mKabEntry->mPhone + "<br>";
            text += "<b>" + i18n( "Mobile Entry:" ) + "</b><br>";
            text += "  " + mobileEntry->mName + " " + mobileEntry->mPhone;
            text += "</qt>";
      
            QMessageBox *msg =
	      new QMessageBox( i18n( "Conflicting Entries" ), text,
	                       QMessageBox::Warning, 1, 2, 0, this );
            msg->setButtonText( 1, i18n( "Use Kab Entry" ) );
            msg->setButtonText( 2, i18n( "Use Mobile Entry" ) );
	  
	    switch ( msg->exec() ) {
	      case 1:
	        // Use KDE Address Book Entry
  	        mobileEntry->mPhone = theCommonEntry->mKabEntry->mPhone;
	        mobileEntry->mName = theCommonEntry->mKabEntry->mName;
	        mobileEntry->mOn = true;
	        mobileEntry->mToBeUpdated = true;
	      
	        setMobState( MODIFIED );
	        break;
	      
  	      case 2:
	        // Use Mobile Address Book Entry
	        theCommonEntry->mKabEntry->mPhone = mobileEntry->mPhone;
	        theCommonEntry->mKabEntry->mName = mobileEntry->mName;
	        theCommonEntry->mKabEntry->mOn = true;
		theCommonEntry->mKabEntry->mToBeUpdated = true;
		
		mobileEntry->mOn = false;
	      
	        setKabState( MODIFIED );
	        break;
 	    }
	  }
        }
      }

    if ( !equivalentEntryFound && mobileEntry->mOn ) {
      // No equivalent entry exists; generate a new one.
      mSyncer->mCommonEntries.append(
        new SyncEntryCommon( true, 0, mobileEntry ) );
      mobileEntry->mOn = false;
    }
  }


  //
  // Create new KAB and Mobile Entries
  //
  
  for ( i = 0; i < mSyncer->mCommonEntries.count(); i++ ) {
    SyncEntryCommon *entry = mSyncer->mCommonEntries.at( i );
    SyncEntryKab *kabEntry = entry->mKabEntry;
    SyncEntryMobile *mobileEntry = entry->mMobileEntry;


    if ( kabEntry && !mobileEntry ) {
      // Create Mobile Entry
      entry->mMobileEntry = new SyncEntryMobile( true, "", kabEntry->mPhone,
                                                 kabEntry->mName );
      entry->mMobileEntry->mToBeInserted = true;
      mSyncer->mMobileEntries.append( entry->mMobileEntry );

      setMobState( MODIFIED );
    } else
    if ( mobileEntry && !kabEntry ) {
      // Create KAB Entry
      entry->mKabEntry = new SyncEntryKab( true, mobileEntry->mIndex,
                                           mobileEntry->mName,
					   mobileEntry->mPhone );
      entry->mKabEntry->mToBeInserted = true;
      mSyncer->mKabEntries.append( entry->mKabEntry );

      setKabState( MODIFIED );
    }
  }


  //
  // Update GUI
  //

  updateKabBook();
  updateMobileBook();

  emit transientStatusMessage( i18n( "Synced phonebooks." ) );
  PushButton8_3->setEnabled( true );
}


void MobileGui::syncPhonebooks()
{
  PushButton8_3->setEnabled( false );

  if ( mKabState == UNLOADED )
    readKabc();
  if ( mMobState == UNLOADED ) {
    mComingFromSyncPhonebooks = true;
    readPhonebook();
  } else
    mergePhonebooks();
}


void MobileGui::updateKabBook()
{
  mKabBook->clear();
  
  for ( uint i = 0; i < mSyncer->mKabEntries.count(); i++ ) {
    SyncEntryKab *kabEntry = mSyncer->mKabEntries.at( i );
    PhoneBookItem *item = new PhoneBookItem( mKabBook, kabEntry,
                                             kabEntry->mName, kabEntry->mPhone,
					     kabEntry->mIndex );
    item->setOn( kabEntry->mOn );
  }
}


void MobileGui::updateMobileBook()
{
  mMobileBook->clear();
  
  for ( uint i = 0; i < mSyncer->mMobileEntries.count(); i++ ) {
    SyncEntryMobile *entry = mSyncer->mMobileEntries.at( i );
    
    if ( !entry->mToBeDeleted ) {
      PhoneBookItem *item = new PhoneBookItem( mMobileBook, entry, entry->mName,
                                               entry->mPhone, entry->mIndex );
      item->setOn( entry->mOn );
    }
  }
}


void MobileGui::toggleConnection()
{
  if ( mConnectButton->text() == i18n( "Connect" ) ) {
    emit connectModem();
    
    readModelInformation();
    refreshStatus();
    
    mConnectButton->setText( tr2i18n( "Disconnect" ) );
    PushButton1->setEnabled( true );
    PushButton5_3->setEnabled( true );

    mABTab->setEnabled( true );
    setKabState( UNLOADED );
    setMobState( UNLOADED );

    ((MobileMain *) mparent)->statusBar()->changeItem( i18n(" Connected "), 1 );
  } else {
    warnKabState( UNLOADED );

    mComingFromToggleConnection = true;
    if ( !warnMobState( UNLOADED ) ) {
      mComingFromToggleConnection = false;
      disconnectGUI();
    }
  }
}


void MobileGui::disconnectGUI()
{
  emit disconnectModem();
    
  mManufacturerLabel->setText( "x" );
  mModelLabel->setText( "x" );
  mGSMVersionLabel->setText( "x" );
  mSerialNumberLabel->setText( "x" );

  mBatteryChargeLabel->setText( "xx %" );
  mSignalQualityLabel->setText( "x" );

  mConnectButton->setText( tr2i18n( "Connect" ) );
  PushButton1->setEnabled( false );
  PushButton5_3->setEnabled( false );
    
  mKabBook->clear();
  mMobileBook->clear();
  setKabState( UNLOADED );
  setMobState( UNLOADED );

  mABTab->setEnabled( false );
  
  mMobHasFD = false;
  mMobHasLD = false;
  mMobHasME = false;
  mMobHasMT = false;
  mMobHasTA = false;
  mMobHasOW = false;
  mMobHasMC = false;
  mMobHasRC = false;
  
  mPBIndexOccupied.resize( 0, false );

  ((MobileMain *) mparent)->statusBar()->changeItem( i18n(" Disconnected "),
                                                     1 );
}


void MobileGui::termAddOutput( const char *line )
{
  mTermIO->append( line );
  mTermIO->setCursorPosition( mTermIO->paragraphs() - 1, 0 );
}


void MobileGui::setKabState( ABState newState )
{
  switch ( mKabState ) {
    case UNLOADED:
      groupBox3->setTitle( tr2i18n( "KDE Address Book" ) );
      mReadKabButton->setEnabled( true );
      PushButton8->setEnabled( false );
      break;
      
    case LOADED:
      if ( newState == MODIFIED ) {
        groupBox3->setTitle( tr2i18n( "KDE Address Book (modified)" ) );
	mReadKabButton->setEnabled( true );
	PushButton8->setEnabled( true );
      } else
      if ( newState == UNLOADED ) {
        groupBox3->setTitle( tr2i18n( "KDE Address Book" ) );
	mReadKabButton->setEnabled( true );
	PushButton8->setEnabled( false );
      }
      break;

    case MODIFIED:
      if ( newState != MODIFIED ) {
        groupBox3->setTitle( tr2i18n( "KDE Address Book" ) );
	mReadKabButton->setEnabled( true );
	PushButton8->setEnabled( false );
      }
      break;
  }
  
  mKabState = newState;
}


void MobileGui::warnKabState( ABState newState )
{
  if ( ( mKabState == MODIFIED ) && ( newState != MODIFIED ) ) {
    QString text = "<qt><b>" + i18n( "Warning" ) + "</b><br>";
    text += i18n( "The KDE address book contains unsaved changes." ) +
            "<br></qt>";
      
    QMessageBox *msg = new QMessageBox( i18n( "Unsaved Changes" ), text,
                                        QMessageBox::Critical, 1, 2, 0, this );
    msg->setButtonText( 1, i18n( "Save" ) );
    msg->setButtonText( 2, i18n( "Discard" ) );
	  
    switch ( msg->exec() ) {
      case 1:
        // Save Changes first
	writeKabc();
	break;
	      
      case 2:
	break;
    }
  }
}


void MobileGui::setMobState( ABState newState )
{
  switch ( mMobState ) {
    case UNLOADED:
      if ( newState == UNLOADED ) {
        groupBox4->setTitle( tr2i18n( "Mobile Phone Book" ) );
        PushButton3->setEnabled( true );
        PushButton12->setEnabled( false );
        PushButton4_2->setEnabled( false );
	MobDeleteButton->setEnabled( false );
      } else
      if ( newState == LOADED ) {
        groupBox4->setTitle( tr2i18n( "Mobile Phone Book" ) );
        PushButton3->setEnabled( true );
        PushButton12->setEnabled( false );
        PushButton4_2->setEnabled( true );
	MobDeleteButton->setEnabled( true );
      }
      break;
      
    case LOADED:
      if ( newState == MODIFIED ) {
        groupBox4->setTitle( tr2i18n( "Mobile Phone Book (modified)" ) );
	PushButton3->setEnabled( true );
	PushButton12->setEnabled( true );
	PushButton4_2->setEnabled( true );
	MobDeleteButton->setEnabled( true );
      } else
      if ( newState == UNLOADED ) {
        groupBox4->setTitle( tr2i18n( "Mobile Phone Book" ) );
	PushButton3->setEnabled( true );
	PushButton12->setEnabled( false );
	PushButton4_2->setEnabled( false );
	MobDeleteButton->setEnabled( false );
      }
      break;

    case MODIFIED:
      if ( newState == UNLOADED ) {
        groupBox4->setTitle( tr2i18n( "Mobile Phone Book" ) );
	PushButton3->setEnabled( true );
	PushButton12->setEnabled( false );
	PushButton4_2->setEnabled( false );
	MobDeleteButton->setEnabled( false );
      } else
      if ( newState == LOADED ) {
        groupBox4->setTitle( tr2i18n( "Mobile Phone Book" ) );
	PushButton3->setEnabled( true );
	PushButton12->setEnabled( false );
	PushButton4_2->setEnabled( true );
	MobDeleteButton->setEnabled( true );
      }
      break;
  }
  
  mMobState = newState;
}


bool MobileGui::warnMobState( ABState newState )
{
  if ( ( mMobState == MODIFIED ) && ( newState != MODIFIED ) )
  {
    QString text = "<qt><b>" + i18n( "Warning" ) + "</b><br>";
    text += i18n( "The mobile phone book contains unsaved changes." ) +
            "<br></qt>";
      
    QMessageBox *msg = new QMessageBox( i18n( "Unsaved Changes" ), text,
                                        QMessageBox::Critical, 1, 2, 0, this );
    msg->setButtonText( 1, i18n( "Save" ) );
    msg->setButtonText( 2, i18n( "Discard" ) );
	  
    switch ( msg->exec() ) {
      case 1:
        // Save Changes first
	writePhonebook();
	return true;
	break;
	      
      case 2:
        return false;
	break;
    }
  }
  
  return false;
}
