/*
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <config.h> // for KDEPIM_NEW_DISTRLISTS

#include "koattendeeeditor.h"
#include "koprefs.h"
#include "koglobals.h"

#ifndef KORG_NOKABC
#include <kabc/addresseedialog.h>
#include <libkdepim/addressesdialog.h>
#include <libkdepim/addresseelineedit.h>
#endif

#include <libkcal/incidence.h>

#include <libemailfunctions/email.h>

#ifdef KDEPIM_NEW_DISTRLISTS
#include "distributionlist.h"
#else
#include <kabc/distributionlist.h>
#endif
#include <kabc/stdaddressbook.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>

using namespace KCal;

KOAttendeeEditor::KOAttendeeEditor( TQWidget * parent, const char *name ) :
    TQWidget( parent, name ),
    mDisableItemUpdate( true )
{
}

void KOAttendeeEditor::initOrganizerWidgets(TQWidget * parent, TQBoxLayout * layout)
{
  mOrganizerHBox = new TQHBox( parent );
  layout->addWidget( mOrganizerHBox );
  // If creating a new event, then the user is the organizer -> show the
  // identity combo
  // readEvent will delete it and set another label text instead, if the user
  // isn't the organizer.
  // Note that the i18n text below is duplicated in readEvent
  TQString whatsThis = i18n("Sets the identity corresponding to "
                           "the organizer of this to-do or event. "
                           "Identities can be set in the 'Personal' "
                           "section of the KOrganizer configuration, or in the "
                           "'Security & Privacy'->'Password & User Account' "
                           "section of the KDE Control Center. In addition, "
                           "identities are gathered from your KMail settings "
                           "and from your address book. If you choose "
                           "to set it globally for KDE in the Control Center, "
                           "be sure to check 'Use email settings from "
                           "Control Center' in the 'Personal' section of the "
                           "KOrganizer configuration.");
  mOrganizerLabel = new TQLabel( i18n( "Identity as organizer:" ),
                                mOrganizerHBox );
  mOrganizerCombo = new TQComboBox( mOrganizerHBox );
  TQWhatsThis::add( mOrganizerLabel, whatsThis );
  TQWhatsThis::add( mOrganizerCombo, whatsThis );
  fillOrganizerCombo();
  mOrganizerHBox->setStretchFactor( mOrganizerCombo, 100 );
}

void KOAttendeeEditor::initEditWidgets(TQWidget * parent, TQBoxLayout * layout)
{
  TQGridLayout *topLayout = new TQGridLayout();
  layout->addLayout( topLayout );

  TQString whatsThis = i18n("Edits the name of the attendee selected in the list "
                           "above, or adds a new attendee if there are no attendees"
                           "in the list.");
  TQLabel *attendeeLabel = new TQLabel( parent );
  TQWhatsThis::add( attendeeLabel, whatsThis );
  attendeeLabel->setText( i18n("Na&me:") );
  topLayout->addWidget( attendeeLabel, 0, 0 );

  mNameEdit = new KPIM::AddresseeLineEdit( parent );
  TQWhatsThis::add( mNameEdit, whatsThis );
  mNameEdit->setClickMessage( i18n("Click to add a new attendee") );
  attendeeLabel->setBuddy( mNameEdit );
  mNameEdit->installEventFilter( this );
  connect( mNameEdit, TQT_SIGNAL( textChanged( const TQString & ) ),
           TQT_SLOT( updateAttendee() ) );
  connect( mNameEdit, TQT_SIGNAL(returnPressed()), TQT_SLOT(expandAttendee()) );
  topLayout->addMultiCellWidget( mNameEdit, 0, 0, 1, 2 );

  whatsThis = i18n("Edits the role of the attendee selected "
                   "in the list above.");
  TQLabel *attendeeRoleLabel = new TQLabel( parent );
  TQWhatsThis::add( attendeeRoleLabel, whatsThis );
  attendeeRoleLabel->setText( i18n("Ro&le:") );
  topLayout->addWidget( attendeeRoleLabel, 1, 0 );

  mRoleCombo = new TQComboBox( false, parent );
  TQWhatsThis::add( mRoleCombo, whatsThis );
  mRoleCombo->insertStringList( Attendee::roleList() );
  attendeeRoleLabel->setBuddy( mRoleCombo );
  connect( mRoleCombo, TQT_SIGNAL( activated( int ) ),
           TQT_SLOT( updateAttendee() ) );
  topLayout->addWidget( mRoleCombo, 1, 1 );

  mDelegateLabel = new TQLabel( parent );
  topLayout->addWidget( mDelegateLabel, 1, 2 );

  whatsThis = i18n("Edits the current attendance status of the attendee "
                   "selected in the list above.");
  TQLabel *statusLabel = new TQLabel( parent );
  TQWhatsThis::add( statusLabel, whatsThis );
  statusLabel->setText( i18n("Stat&us:") );
  topLayout->addWidget( statusLabel, 2, 0 );

  mStatusCombo = new TQComboBox( false, parent );
  TQWhatsThis::add( mStatusCombo, whatsThis );
//   mStatusCombo->insertStringList( Attendee::statusList() );
  mStatusCombo->insertItem( SmallIcon( "help" ), Attendee::statusName( Attendee::NeedsAction ) );
  mStatusCombo->insertItem( KOGlobals::self()->smallIcon( "ok" ), Attendee::statusName( Attendee::Accepted ) );
  mStatusCombo->insertItem( KOGlobals::self()->smallIcon( "no" ), Attendee::statusName( Attendee::Declined ) );
  mStatusCombo->insertItem( KOGlobals::self()->smallIcon( "apply" ), Attendee::statusName( Attendee::Tentative ) );
  mStatusCombo->insertItem( KOGlobals::self()->smallIcon( "mail_forward" ), Attendee::statusName( Attendee::Delegated ) );
  mStatusCombo->insertItem( Attendee::statusName( Attendee::Completed ) );
  mStatusCombo->insertItem( KOGlobals::self()->smallIcon( "help" ), Attendee::statusName( Attendee::InProcess ) );

  statusLabel->setBuddy( mStatusCombo );
  connect( mStatusCombo, TQT_SIGNAL( activated( int ) ),
           TQT_SLOT( updateAttendee() ) );
  topLayout->addWidget( mStatusCombo, 2, 1 );

  topLayout->setColStretch( 2, 1 );

  mRsvpButton = new TQCheckBox( parent );
  TQWhatsThis::add( mRsvpButton,
		   i18n("Edits whether to send an email to the attendee "
			"selected in the list above to request "
			"a response concerning attendance.") );
  mRsvpButton->setText( i18n("Re&quest response") );
  connect( mRsvpButton, TQT_SIGNAL( clicked() ), TQT_SLOT( updateAttendee() ) );
  topLayout->addWidget( mRsvpButton, 2, 2 );

  TQWidget *buttonBox = new TQWidget( parent );
  TQVBoxLayout *buttonLayout = new TQVBoxLayout( buttonBox );

  mAddButton = new TQPushButton( i18n("&New"), buttonBox );
  TQWhatsThis::add( mAddButton,
		   i18n("Adds a new attendee to the list. Once the "
		   	"attendee is added, you will be able to "
			"edit the attendee's name, role, attendance "
			"status, and whether or not the attendee is required "
			"to respond to the invitation. To select an attendee "
			"from your addressbook, click the 'Select Addressee' "
			"button instead.") );
  buttonLayout->addWidget( mAddButton );
  connect( mAddButton, TQT_SIGNAL( clicked() ), TQT_SLOT( addNewAttendee() ) );

  mRemoveButton = new TQPushButton( i18n("&Remove"), buttonBox );
  TQWhatsThis::add( mRemoveButton,
		   i18n("Removes the attendee selected in "
		   	"the list above.") );
  buttonLayout->addWidget( mRemoveButton );

  mAddressBookButton = new TQPushButton( i18n("Select Addressee..."),
                                        buttonBox );
  TQWhatsThis::add( mAddressBookButton,
		   i18n("Opens your address book, allowing you to select "
			"new attendees from it.") );
  buttonLayout->addWidget( mAddressBookButton );
  connect( mAddressBookButton, TQT_SIGNAL( clicked() ), TQT_SLOT( openAddressBook() ) );

  topLayout->addMultiCellWidget( buttonBox, 0, 3, 3, 3 );

#ifdef KORG_NOKABC
  mAddressBookButton->hide();
#endif
}

void KOAttendeeEditor::openAddressBook()
{
#ifndef KORG_NOKABC
  KPIM::AddressesDialog *dia = new KPIM::AddressesDialog( this, "adddialog" );
  dia->setShowCC( false );
  dia->setShowBCC( false );
  if ( dia->exec() ) {
    KABC::Addressee::List aList = dia->allToAddressesNoDuplicates();
    for ( KABC::Addressee::List::iterator itr = aList.begin();
          itr != aList.end(); ++itr ) {
      insertAttendeeFromAddressee( (*itr) );
    }
  }
  delete dia;
  return;
#endif
}

void KOAttendeeEditor::insertAttendeeFromAddressee(const KABC::Addressee &a, const Attendee * at)
{
  bool myself = KOPrefs::instance()->thatIsMe( a.preferredEmail() );
  bool sameAsOrganizer = mOrganizerCombo &&
  KPIM::compareEmail( a.preferredEmail(), mOrganizerCombo->currentText(), false );
  KCal::Attendee::PartStat partStat = at? at->status() : KCal::Attendee::NeedsAction;
  bool rsvp = at? at->RSVP() : true;

  if ( myself && sameAsOrganizer ) {
    partStat = KCal::Attendee::Accepted;
    rsvp = false;
  }
  Attendee *newAt = new Attendee( a.realName(),
                                  a.preferredEmail(),
                                  !myself, partStat,
                                  at ? at->role() : Attendee::ReqParticipant,
                                  a.uid() );
  newAt->setRSVP( rsvp );
  insertAttendee( newAt, true );
  mnewAttendees.append( newAt );
}

void KOAttendeeEditor::fillOrganizerCombo()
{
  Q_ASSERT( mOrganizerCombo );
  // Get all emails from KOPrefs (coming from various places),
  // and insert them - removing duplicates
  const TQStringList lst = KOPrefs::instance()->fullEmails();
  TQStringList uniqueList;
  for( TQStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( uniqueList.find( *it ) == uniqueList.end() )
      uniqueList << *it;
  }
  mOrganizerCombo->insertStringList( uniqueList );
}

void KOAttendeeEditor::addNewAttendee()
{
  // check if there's still an unchanged example entry, and if so
  // suggest to edit that first
  if ( TQListViewItem* item = hasExampleAttendee() ) {
      KMessageBox::information( this,
          i18n( "Please edit the example attendee, before adding more." ), TQString::null,
          "EditExistingExampleAttendeeFirst" );
      // make sure the example attendee is selected
      item->setSelected( true );
      item->listView()->setCurrentItem( item );
      return;
  }
  Attendee *a = new Attendee( i18n("Firstname Lastname"),
                              i18n("name") + "@example.net", true );
  insertAttendee( a, false );
  mnewAttendees.append( a );
  updateAttendeeInput();
  // We don't want the hint again
  mNameEdit->setClickMessage( "" );
  mNameEdit->setFocus();
  TQTimer::singleShot( 0, mNameEdit, TQT_SLOT( selectAll() ) );
}

void KOAttendeeEditor::readEvent(KCal::Incidence * incidence)
{
  mdelAttendees.clear();
  mnewAttendees.clear();
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) || incidence->organizer().isEmpty() ) {
    if ( !mOrganizerCombo ) {
      mOrganizerCombo = new TQComboBox( mOrganizerHBox );
      fillOrganizerCombo();
    }
    mOrganizerLabel->setText( i18n( "Identity as organizer:" ) );

    int found = -1;
    TQString fullOrganizer = incidence->organizer().fullName();
    for ( int i = 0 ; i < mOrganizerCombo->count(); ++i ) {
      if ( mOrganizerCombo->text( i ) == fullOrganizer ) {
        found = i;
        mOrganizerCombo->setCurrentItem( i );
        break;
      }
    }
    if ( found < 0 ) {
      mOrganizerCombo->insertItem( fullOrganizer, 0 );
      mOrganizerCombo->setCurrentItem( 0 );
    }
  } else { // someone else is the organizer
    if ( mOrganizerCombo ) {
      delete mOrganizerCombo;
      mOrganizerCombo = 0;
    }
    mOrganizerLabel->setText( i18n( "Organizer: %1" ).arg( incidence->organizer().fullName() ) );
  }

  Attendee::List al = incidence->attendees();
  Attendee::List::ConstIterator it;
  Attendee *first = 0;
  for( it = al.begin(); it != al.end(); ++it ) {
    Attendee *a = new Attendee( **it );
    if ( !first ) {
      first = a;
    }
    insertAttendee( a, true );
  }

  // Set the initial editing values to the first attendee in the list.
  if ( first ) {
    // Don't update the item here, the user didn't edit it, so it's not needed.
    // Also, AttendeeEditor's subclasses didn't set the current Item at this point
    // so if updateAttendee is called now what will happen is that a random item
    // will get the text of "first".
    mDisableItemUpdate = true;

    setSelected( 0 );
    mNameEdit->setText( first->fullName() );
    mUid = first->uid();
    mRoleCombo->setCurrentItem( first->role() );
    if ( first->status() != KCal::Attendee::None ) {
      mStatusCombo->setCurrentItem( first->status() );
    } else {
      mStatusCombo->setCurrentItem( KCal::Attendee::NeedsAction );
    }
    mRsvpButton->setChecked( first->RSVP() );
    mRsvpButton->setEnabled( true );
    mDisableItemUpdate = false;
  }
}

void KOAttendeeEditor::writeEvent(KCal::Incidence * incidence)
{
  if ( mOrganizerCombo ) {
    // TODO: Don't take a string and split it up... Is there a better way?
    incidence->setOrganizer( mOrganizerCombo->currentText() );
  }
}

void KOAttendeeEditor::setEnableAttendeeInput(bool enabled)
{
  //mNameEdit->setEnabled( enabled );
  mRoleCombo->setEnabled( enabled );
  mStatusCombo->setEnabled( enabled );
  mRsvpButton->setEnabled( enabled );

  mRemoveButton->setEnabled( enabled );
}

void KOAttendeeEditor::clearAttendeeInput()
{
  mNameEdit->setText("");
  mUid = TQString::null;
  mRoleCombo->setCurrentItem(0);
  mStatusCombo->setCurrentItem(0);
  mRsvpButton->setChecked(true);
  setEnableAttendeeInput( false );
  mDelegateLabel->setText( TQString() );
}

void KOAttendeeEditor::expandAttendee()
{
  KABC::Addressee::List aList = expandDistList( mNameEdit->text() );
  if ( !aList.isEmpty() ) {
    int index = selectedIndex();
    for ( KABC::Addressee::List::iterator itr = aList.begin(); itr != aList.end(); ++itr ) {
      insertAttendeeFromAddressee( (*itr) );
    }
    setSelected( index );
    removeAttendee( currentAttendee() );
  }
}

void KOAttendeeEditor::updateAttendee()
{
  Attendee *a = currentAttendee();
  if ( !a || mDisableItemUpdate )
    return;

  TQString text = mNameEdit->text();
  if ( !mNameEdit->text().startsWith( "\"" ) ) {
    // Quote the text as it might contain commas and other quotable chars.
    text = KPIM::quoteNameIfNecessary( text );
  }

  TQString name, email;
  if ( KPIM::getNameAndMail( text, name, email ) ) {
    name.remove( '"' );
    email.remove( '"' ).remove( '>' );
  } else {
    name = TQString();
    email = mNameEdit->text();
  }

  bool iAmTheOrganizer = mOrganizerCombo &&
    KOPrefs::instance()->thatIsMe( mOrganizerCombo->currentText() );
  if ( iAmTheOrganizer ) {
    bool myself =
      KPIM::compareEmail( email, mOrganizerCombo->currentText(), false );
    bool wasMyself =
      KPIM::compareEmail( a->email(), mOrganizerCombo->currentText(), false );
    if ( myself ) {
      mRsvpButton->setChecked( false );
      mRsvpButton->setEnabled( false );
    } else if ( wasMyself ) {
      // this was me, but is no longer, reset
      mStatusCombo->setCurrentItem( KCal::Attendee::NeedsAction );
      mRsvpButton->setChecked( true );
      mRsvpButton->setEnabled( true );
    }
  }
  a->setName( name );
  a->setUid( mUid );
  a->setEmail( email );
  a->setRole( Attendee::Role( mRoleCombo->currentItem() ) );
  a->setStatus( Attendee::PartStat( mStatusCombo->currentItem() ) );
  a->setRSVP( mRsvpButton->isChecked() );

  updateCurrentItem();
}

void KOAttendeeEditor::fillAttendeeInput( KCal::Attendee *a )
{
  mDisableItemUpdate = true;

  TQString tname, temail;
  TQString username = a->name();
  if ( !a->email().isEmpty() ) {
    username = KPIM::quoteNameIfNecessary( username );

    KPIM::getNameAndMail( username, tname, temail ); // ignore return value
                                                     // which is always false
    tname += " <" + a->email() + '>';
  }

  bool myself = KOPrefs::instance()->thatIsMe( a->email() );
  bool sameAsOrganizer = mOrganizerCombo &&
          KPIM::compareEmail( a->email(),
                                   mOrganizerCombo->currentText(), false );
  KCal::Attendee::PartStat partStat = a->status();
  bool rsvp = a->RSVP();

  if ( myself && sameAsOrganizer && a->status() == KCal::Attendee::None ) {
      partStat = KCal::Attendee::Accepted;
      rsvp = false;
  }

  mNameEdit->setText(tname);
  mUid = a->uid();
  mRoleCombo->setCurrentItem(a->role());
  if ( partStat != KCal::Attendee::None ) {
    mStatusCombo->setCurrentItem( partStat );
  } else {
    mStatusCombo->setCurrentItem( KCal::Attendee::NeedsAction );
  }
  mRsvpButton->setChecked( rsvp );

  mDisableItemUpdate = false;
  setEnableAttendeeInput( true );

  if ( a->status() == Attendee::Delegated ) {
    if ( !a->delegate().isEmpty() )
      mDelegateLabel->setText( i18n( "Delegated to %1" ).arg( a->delegate() ) );
    else if ( !a->delegator().isEmpty() )
      mDelegateLabel->setText( i18n( "Delegated from %1" ).arg( a->delegator() ) );
    else
      mDelegateLabel->setText( i18n( "Not delegated" ) );
  }
  if( myself )
    mRsvpButton->setEnabled( false );

}

void KOAttendeeEditor::updateAttendeeInput()
{
  setEnableAttendeeInput(!mNameEdit->text().isEmpty());
  Attendee* a = currentAttendee();
  if ( a ) {
    fillAttendeeInput( a );
  } else {
    clearAttendeeInput();
  }
}

void KOAttendeeEditor::cancelAttendeeEvent( KCal::Incidence *incidence )
{
  incidence->clearAttendees();

  if ( mdelAttendees.isEmpty() ) {
    return;
  }

  Attendee *att;
  for ( att = mdelAttendees.first(); att; att = mdelAttendees.next() ) {
    bool isNewAttendee = false;
    if ( !mnewAttendees.isEmpty() ) {
      for ( Attendee *newAtt = mnewAttendees.first(); newAtt; newAtt = mnewAttendees.next() ) {
        if ( *att == *newAtt ) {
          isNewAttendee = true;
          break;
        }
      }
    }
    if ( !isNewAttendee ) {
      incidence->addAttendee( new Attendee( *att ) );
    }
  }
  mdelAttendees.clear();
}

void KOAttendeeEditor::acceptForMe()
{
  changeStatusForMe( Attendee::Accepted );
}

void KOAttendeeEditor::declineForMe()
{
  changeStatusForMe( Attendee::Declined );
}

bool KOAttendeeEditor::eventFilter(TQObject *watched, TQEvent *ev)
{
  if ( watched && watched == mNameEdit && ev->type() == TQEvent::FocusIn &&
       currentAttendee() == 0 ) {
    addNewAttendee();
  }

  return TQWidget::eventFilter( watched, ev );
}

bool KOAttendeeEditor::isExampleAttendee( const KCal::Attendee* attendee ) const
{
    if ( !attendee ) return false;
    if ( attendee->name() == i18n( "Firstname Lastname" )
        && attendee->email().endsWith( "example.net" ) ) {
        return true;
    }
    return false;
}

KABC::Addressee::List KOAttendeeEditor::expandDistList( const TQString &text )  const
{
  KABC::Addressee::List aList;
  KABC::AddressBook *abook = KABC::StdAddressBook::self( true );

#ifdef KDEPIM_NEW_DISTRLISTS
  const TQValueList<KPIM::DistributionList::Entry> eList =
    KPIM::DistributionList::findByName( abook, text ).entries( abook );
  TQValueList<KPIM::DistributionList::Entry>::ConstIterator eit;
  for ( eit = eList.begin(); eit != eList.end(); ++eit ) {
    KABC::Addressee a = (*eit).addressee;
    if ( !a.preferredEmail().isEmpty() && aList.find( a ) == aList.end() ) {
      aList.append( a ) ;
    }
  }

#else
  KABC::DistributionListManager manager( abook );
  manager.load();
  const TQStringList dList = manager.listNames();
  for ( TQStringList::ConstIterator it = dList.begin(); it != dList.end(); ++it ) {
    if ( (*it) == text ) {
      const TQValueList<KABC::DistributionList::Entry> eList = manager.list( *it )->entries();
      TQValueList<KABC::DistributionList::Entry>::ConstIterator eit;
      for ( eit = eList.begin(); eit != eList.end(); ++eit ) {
        KABC::Addressee a = (*eit).addressee;
        if ( !a.preferredEmail().isEmpty() && aList.find( a ) == aList.end() ) {
          aList.append( a ) ;
        }
      }
    }
  }
#endif
  return aList;
}


#include "koattendeeeditor.moc"
