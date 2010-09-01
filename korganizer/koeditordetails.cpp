/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "koeditordetails.h"

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqdatetime.h>
#include <tqdragobject.h>
#include <tqfiledialog.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqregexp.h>
#include <tqtooltip.h>
#include <tqvbox.h>
#include <tqvgroupbox.h>
#include <tqwhatsthis.h>
#include <tqwidgetstack.h>
#include <tqvaluevector.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#ifndef KORG_NOKABC
#include <kabc/addresseedialog.h>
#include <kabc/vcardconverter.h>
#include <libkdepim/addressesdialog.h>
#include <libkdepim/addresseelineedit.h>
#include <libkdepim/distributionlist.h>
#include <kabc/stdaddressbook.h>
#endif
#include <libkdepim/kvcarddrag.h>
#include <libemailfunctions/email.h>

#include <libkcal/incidence.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koeditorfreebusy.h"

#include "kocore.h"

template <>
CustomListViewItem<KCal::Attendee *>::~CustomListViewItem()
{
  // do not delete mData here
//  delete mData;
}

template <>
void CustomListViewItem<KCal::Attendee *>::updateItem()
{
  setText(0,mData->name());
  setText(1,mData->email());
  setText(2,mData->roleStr());
  setText(3,mData->statusStr());
  if (mData->RSVP() && !mData->email().isEmpty())
    setPixmap(4,KOGlobals::self()->smallIcon("mailappt"));
  else
    setPixmap(4,KOGlobals::self()->smallIcon("nomailappt"));
  setText(5, mData->delegate());
  setText(6, mData->delegator());
}

KOAttendeeListView::KOAttendeeListView ( TQWidget *parent, const char *name )
  : KListView(parent, name)
{
  setAcceptDrops( true );
  setAllColumnsShowFocus( true );
  setSorting( -1 );
}

/** KOAttendeeListView is a child class of KListView  which supports
 *  dropping of attendees (e.g. from kaddressbook) onto it. If an attendeee
 *  was dropped, the signal dropped(Attendee*)  is emitted. Valid drop classes
 *   are KVCardDrag and TQTextDrag.
 */
KOAttendeeListView::~KOAttendeeListView()
{
}

void KOAttendeeListView::contentsDragEnterEvent( TQDragEnterEvent *e )
{
  dragEnterEvent(e);
}

void KOAttendeeListView::contentsDragMoveEvent( TQDragMoveEvent *e )
{
#ifndef KORG_NODND
  if ( KVCardDrag::canDecode( e ) || TQTextDrag::canDecode( e ) ) {
    e->accept();
  } else {
    e->ignore();
  }
#endif
}

void KOAttendeeListView::dragEnterEvent( TQDragEnterEvent *e )
{
#ifndef KORG_NODND
  if ( KVCardDrag::canDecode( e ) || TQTextDrag::canDecode( e ) ) {
    e->accept();
  } else {
    e->ignore();
  }
#endif
}

void KOAttendeeListView::addAttendee( const TQString &newAttendee )
{
  kdDebug(5850) << " Email: " << newAttendee << endl;
  TQString name;
  TQString email;
  KPIM::getNameAndMail( newAttendee, name, email );
  emit dropped( new Attendee( name, email, true ) );
}

void KOAttendeeListView::contentsDropEvent( TQDropEvent *e )
{
  dropEvent(e);
}

void KOAttendeeListView::dropEvent( TQDropEvent *e )
{
#ifndef KORG_NODND
  TQString text;

#ifndef KORG_NOKABC
  KABC::Addressee::List list;
  if ( KVCardDrag::decode( e, list ) ) {
    KABC::Addressee::List::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      TQString em( (*it).fullEmail() );
      if ( em.isEmpty() ) {
        em = (*it).realName();
      }
      addAttendee( em );
    }
  } else
#endif // KORG_NOKABC
  if (TQTextDrag::decode(e,text)) {
    kdDebug(5850) << "Dropped : " << text << endl;
    TQStringList emails = TQStringList::split(",",text);
    for(TQStringList::ConstIterator it = emails.begin();it!=emails.end();++it) {
      addAttendee(*it);
    }
  }
#endif //KORG_NODND
}


KOEditorDetails::KOEditorDetails( int spacing, TQWidget *parent,
                                  const char *name )
  : KOAttendeeEditor( parent, name), mDisableItemUpdate( false )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( spacing );

  initOrganizerWidgets( this, topLayout );

  mListView = new KOAttendeeListView( this, "mListView" );
  TQWhatsThis::add( mListView,
		   i18n("Displays information about current attendees. "
		   	"To edit an attendee, select it in this list "
			"and modify the values in the area below. "
		   	"Clicking on a column title will sort the list "
			"according to that column. The RSVP column "
			"indicates whether or not a response is requested "
			"from the attendee.") );
  mListView->addColumn( i18n("Name"), 200 );
  mListView->addColumn( i18n("Email"), 200 );
  mListView->addColumn( i18n("Role"), 80 );
  mListView->addColumn( i18n("Status"), 100 );
  mListView->addColumn( i18n("RSVP"), 55 );
  mListView->addColumn( i18n("Delegated to"), 120 );
  mListView->addColumn( i18n("Delegated from" ), 120 );
  mListView->setResizeMode( TQListView::LastColumn );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    mListView->setFixedHeight( 78 );
  }

  connect( mListView, TQT_SIGNAL( selectionChanged( TQListViewItem * ) ),
           TQT_SLOT( updateAttendeeInput() ) );
#ifndef KORG_NODND
  connect( mListView, TQT_SIGNAL( dropped( Attendee * ) ),
           TQT_SLOT( slotInsertAttendee( Attendee * ) ) );
#endif
  topLayout->addWidget( mListView );

  initEditWidgets( this, topLayout );

  connect( mRemoveButton, TQT_SIGNAL(clicked()), TQT_SLOT(removeAttendee()) );

  updateAttendeeInput();
}

KOEditorDetails::~KOEditorDetails()
{
}

bool KOEditorDetails::hasAttendees()
{
  return mListView->childCount() > 0;
}

void KOEditorDetails::removeAttendee()
{
  AttendeeListItem *aItem =
      static_cast<AttendeeListItem *>( mListView->selectedItem() );
  if ( !aItem ) return;

  AttendeeListItem *nextSelectedItem = static_cast<AttendeeListItem*>( aItem->nextSibling() );
  if( mListView->childCount() == 1 )
      nextSelectedItem = 0;
  if( mListView->childCount() > 1 && aItem == mListView->lastItem() )
      nextSelectedItem = static_cast<AttendeeListItem*>(  mListView->firstChild() );

  Attendee *attendee = aItem->data();
  Attendee *delA = new Attendee( attendee->name(), attendee->email(),
                                 attendee->RSVP(), attendee->status(),
                                 attendee->role(), attendee->uid() );
  mdelAttendees.append( delA );
  delete aItem;

  if( nextSelectedItem ) {
      mListView->setSelected( nextSelectedItem, true );
  }
  updateAttendeeInput();
  emit updateAttendeeSummary( mListView->childCount() );
}


void KOEditorDetails::insertAttendee( Attendee *a, bool goodEmailAddress )
{
  Q_UNUSED( goodEmailAddress );

  // lastItem() is O(n), but for n very small that should be fine
  AttendeeListItem *item = new AttendeeListItem(
    a, mListView, static_cast<KListViewItem*>( mListView->lastItem() ) );
  mListView->setSelected( item, true );
  emit updateAttendeeSummary( mListView->childCount() );
}

void KOEditorDetails::removeAttendee( Attendee *attendee )
{
  TQListViewItem *item;
  for ( item = mListView->firstChild(); item;  item = item->nextSibling() ) {
    AttendeeListItem *anItem = static_cast<AttendeeListItem *>( item );
    Attendee *att = anItem->data();
    if ( att == attendee ) {
      delete anItem;
      break;
    }
  }
}

void KOEditorDetails::setDefaults()
{
  mRsvpButton->setChecked( true );
}

void KOEditorDetails::readEvent( Incidence *event )
{
  mListView->clear();
  KOAttendeeEditor::readEvent( event );

  mListView->setSelected( mListView->firstChild(), true );

  emit updateAttendeeSummary( mListView->childCount() );
}

void KOEditorDetails::writeEvent(Incidence *event)
{
  event->clearAttendees();
  TQValueVector<TQListViewItem*> toBeDeleted;
  TQListViewItem *item;
  AttendeeListItem *a;
  for (item = mListView->firstChild(); item;
       item = item->nextSibling()) {
    a = (AttendeeListItem *)item;
    Attendee *attendee = a->data();
    Q_ASSERT( attendee );
    /* Check if the attendee is a distribution list and expand it */
    if ( attendee->email().isEmpty() ) {
      KPIM::DistributionList list =
        KPIM::DistributionList::findByName( KABC::StdAddressBook::self(), attendee->name() );
      if ( !list.isEmpty() ) {
        toBeDeleted.push_back( item ); // remove it once we are done expanding
        KPIM::DistributionList::Entry::List entries = list.entries( KABC::StdAddressBook::self() );
        KPIM::DistributionList::Entry::List::Iterator it( entries.begin() );
        while ( it != entries.end() ) {
          KPIM::DistributionList::Entry &e = ( *it );
          ++it;
          // this calls insertAttendee, which appends
          insertAttendeeFromAddressee( e.addressee, attendee );
          // TODO: duplicate check, in case it was already added manually
        }
      }
    } else {
      bool skip = false;
      if ( attendee->email().endsWith( "example.net" ) ) {
        if ( KMessageBox::warningYesNo( this, i18n("%1 does not look like a valid email address. "
                "Are you sure you want to invite this participant?").arg( attendee->email() ),
              i18n("Invalid email address") ) != KMessageBox::Yes ) {
          skip = true;
        }
      }
      if ( !skip ) {
        event->addAttendee( new Attendee( *attendee ) );
      }
    }
  }

  KOAttendeeEditor::writeEvent( event );

  // cleanup
  TQValueVector<TQListViewItem*>::iterator it;
  for( it = toBeDeleted.begin(); it != toBeDeleted.end(); ++it ) {
    delete *it;
  }
}

bool KOEditorDetails::validateInput()
{
  return true;
}

KCal::Attendee * KOEditorDetails::currentAttendee() const
{
  TQListViewItem *item = mListView->selectedItem();
  AttendeeListItem *aItem = static_cast<AttendeeListItem *>( item );
  if ( !aItem )
    return 0;
  return aItem->data();
}

void KOEditorDetails::updateCurrentItem()
{
  AttendeeListItem *item = static_cast<AttendeeListItem*>( mListView->selectedItem() );
  if ( item )
    item->updateItem();
}

void KOEditorDetails::slotInsertAttendee( Attendee *a )
{
  insertAttendee( a );
  mnewAttendees.append( a );
}

void KOEditorDetails::setSelected( int index )
{
  int count = 0;
  for ( TQListViewItemIterator it( mListView ); it.current(); ++it ) {
    if ( count == index ) {
      mListView->setSelected( *it, true );
      return;
    }
    count++;
  }
}

int KOEditorDetails::selectedIndex()
{
  int index = 0;
  for ( TQListViewItemIterator it( mListView ); it.current(); ++it ) {
    if ( mListView->isSelected( *it ) ) {
      break;
    }
    index++;
  }
  return index;
}

void KOEditorDetails::changeStatusForMe(Attendee::PartStat status)
{
  const TQStringList myEmails = KOPrefs::instance()->allEmails();
  for ( TQListViewItemIterator it( mListView ); it.current(); ++it ) {
    AttendeeListItem *item = static_cast<AttendeeListItem*>( it.current() );
    for ( TQStringList::ConstIterator it2( myEmails.begin() ), end( myEmails.end() ); it2 != end; ++it2 ) {
      if ( item->data()->email() == *it2 ) {
        item->data()->setStatus( status );
        item->updateItem();
      }
    }
  }
}

TQListViewItem* KOEditorDetails::hasExampleAttendee() const
{
  for ( TQListViewItemIterator it( mListView ); it.current(); ++it ) {
    AttendeeListItem *item = static_cast<AttendeeListItem*>( it.current() );
    Attendee *attendee = item->data();
    Q_ASSERT( attendee );
    if ( isExampleAttendee( attendee ) )
        return item;
  }
  return 0;
}

#include "koeditordetails.moc"
