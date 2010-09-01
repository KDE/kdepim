/*
    This file is part of the KOrganizer alarm daemon.

    Copyright (c) 2000,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2009-2010 Klar�lvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include <tqhbox.h>
#include <tqvbox.h>
#include <tqlabel.h>
#include <tqfile.h>
#include <tqspinbox.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqcstring.h>
#include <tqdatastream.h>
#include <tqsplitter.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdcopservicestarter.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kprocess.h>
#include <kaudioplayer.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kcombobox.h>
#include <klistview.h>
#include <kwin.h>
#include <klockfile.h>

#include <libkcal/event.h>
#include <libkcal/incidenceformatter.h>

#include "koeventviewer.h"

#include "alarmdialog.h"
#include "alarmdialog.moc"

static int defSuspendVal = 5;
static int defSuspendUnit = 0; // 0=>minutes, 1=>hours, 2=>days, 3=>weeks

class AlarmListItem : public KListViewItem
{
  public:
    AlarmListItem( const TQString &uid, TQListView *parent )
      : KListViewItem( parent ), mUid( uid ), mNotified( false )
    {
    }

    ~AlarmListItem()
    {
    }

    int compare( TQListViewItem *item, int iCol, bool bAscending ) const;

    TQString mDisplayText;

    TQString mUid;
    TQDateTime mRemindAt;
    TQDateTime mHappening;
    bool mNotified;
};

int AlarmListItem::compare( TQListViewItem *item, int iCol, bool bAscending ) const
{
  if ( iCol == 1 ) {
    AlarmListItem *pItem = static_cast<AlarmListItem *>( item );
    return pItem->mHappening.secsTo( mHappening );
  } else {
    return KListViewItem::compare( item, iCol, bAscending );
  }
}

typedef TQValueList<AlarmListItem*> ItemList;

AlarmDialog::AlarmDialog( KCal::CalendarResources *calendar, TQWidget *parent, const char *name )
  : KDialogBase( Plain,
                 WType_TopLevel | WStyle_Customize | WStyle_StaysOnTop | WStyle_DialogBorder,
                 parent, name, false, i18n("Reminder"),
                 Ok | User1 | User2 | User3, NoDefault,
                 false, i18n("Edit..."), i18n("Dismiss All"), i18n("Dismiss Reminder") ),
                 mCalendar( calendar ), mSuspendTimer(this)
{
  // User1 => Edit...
  // User2 => Dismiss All
  // User3 => Dismiss Selected
  //    Ok => Suspend

  connect( calendar, TQT_SIGNAL(calendarChanged()),
           this, TQT_SLOT(slotCalendarChanged()) );

  KGlobal::iconLoader()->addAppDir( "kdepim" );
  setButtonOK( i18n( "Suspend" ) );

  TQWidget *topBox = plainPage();
  TQBoxLayout *topLayout = new TQVBoxLayout( topBox );
  topLayout->setSpacing( spacingHint() );

  TQLabel *label = new TQLabel( i18n("The following items triggered reminders:"), topBox );
  topLayout->addWidget( label );

  mSplitter = new TQSplitter( TQt::Vertical, topBox );
  mSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
  topLayout->addWidget( mSplitter );

  mIncidenceListView = new KListView( mSplitter );
  mIncidenceListView->addColumn( i18n( "Summary" ) );
  mIncidenceListView->addColumn( i18n( "Date, Time" ) );
  mIncidenceListView->setSorting( 0, true );
  mIncidenceListView->setSorting( 1, true );
  mIncidenceListView->setSortColumn( 1 );
  mIncidenceListView->setShowSortIndicator( true );
  mIncidenceListView->setAllColumnsShowFocus( true );
  mIncidenceListView->setSelectionMode( TQListView::Extended );
  connect( mIncidenceListView, TQT_SIGNAL(selectionChanged()), TQT_SLOT(updateButtons()) );
  connect( mIncidenceListView, TQT_SIGNAL(doubleClicked(TQListViewItem*)), TQT_SLOT(edit()) );
  connect( mIncidenceListView, TQT_SIGNAL(currentChanged(TQListViewItem*)), TQT_SLOT(showDetails()) );
  connect( mIncidenceListView, TQT_SIGNAL(selectionChanged()), TQT_SLOT(showDetails()) );

  mDetailView = new KOEventViewer( mCalendar, mSplitter );
  mDetailView->setFocus(); // set focus here to start with to make it harder
                           // to hit return by mistake and dismiss a reminder.

  TQHBox *suspendBox = new TQHBox( topBox );
  suspendBox->setSpacing( spacingHint() );
  topLayout->addWidget( suspendBox );

  TQLabel *l = new TQLabel( i18n("Suspend &duration:"), suspendBox );
  mSuspendSpin = new TQSpinBox( 1, 9999, 1, suspendBox );
  mSuspendSpin->setValue( defSuspendVal );  // default suspend duration
  l->setBuddy( mSuspendSpin );

  mSuspendUnit = new KComboBox( suspendBox );
  mSuspendUnit->insertItem( i18n("minute(s)") );
  mSuspendUnit->insertItem( i18n("hour(s)") );
  mSuspendUnit->insertItem( i18n("day(s)") );
  mSuspendUnit->insertItem( i18n("week(s)") );
  mSuspendUnit->setCurrentItem( defSuspendUnit );

  connect( &mSuspendTimer, TQT_SIGNAL(timeout()), TQT_SLOT(wakeUp()) );

  setMainWidget( mIncidenceListView );
  mIncidenceListView->setMinimumSize( 500, 50 );

  readLayout();
}

AlarmDialog::~AlarmDialog()
{
  mIncidenceListView->clear();
}

AlarmListItem *AlarmDialog::searchByUid( const TQString &uid )
{
  AlarmListItem *found = 0;
  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ) {
    AlarmListItem *item = static_cast<AlarmListItem*>( it.current() );
    if ( item->mUid == uid ) {
      found = item;
      break;
    }
    ++it;
  }
  return found;
}

static TQString etc = i18n( "elipsis", "..." );
static TQString cleanSummary( const TQString &summary )
{
  uint maxLen = 45;
  TQString retStr = summary;
  retStr.replace( '\n', ' ' );
  if ( retStr.length() > maxLen ) {
    maxLen -= etc.length();
    retStr = retStr.left( maxLen );
    retStr += etc;
  }
  return retStr;
}

void AlarmDialog::readLayout()
{
  KConfig *config = kapp->config();
  config->setGroup( "Layout" );
  TQValueList<int> sizes = config->readIntListEntry( "SplitterSizes" );
  if ( sizes.count() == 2 ) {
    mSplitter->setSizes( sizes );
  }
  mSplitter->setCollapsible( mIncidenceListView, false );
  mSplitter->setCollapsible( mDetailView, false );
}

void AlarmDialog::writeLayout()
{
  KConfig *config = kapp->config();
  config->setGroup( "Layout" );
  TQValueList<int> list = mSplitter->sizes();
  config->writeEntry( "SplitterSizes", list );
}

void AlarmDialog::addIncidence( Incidence *incidence,
                                const TQDateTime &reminderAt,
                                const TQString &displayText )
{
  AlarmListItem *item = searchByUid( incidence->uid() );
  if ( !item ) {
    item = new AlarmListItem( incidence->uid(), mIncidenceListView );
  }
  item->mNotified = false;
  item->mHappening = TQDateTime();
  item->mRemindAt = reminderAt;
  item->mDisplayText = displayText;
  item->setText( 0, cleanSummary( incidence->summary() ) );
  item->setText( 1, TQString() );

  TQString displayStr;
  const TQDateTime dateTime = triggerDateForIncidence( incidence, reminderAt, displayStr );

  item->mHappening = dateTime;
  item->setText( 1, displayStr );

  if ( incidence->type() == "Event" ) {
    item->setPixmap( 0, SmallIcon( "appointment" ) );
  } else {
    item->setPixmap( 0, SmallIcon( "todo" ) );
  }

  if ( activeCount() == 1 ) { // previously empty
    mIncidenceListView->clearSelection();
    item->setSelected( true );
  }
  showDetails();
}

void AlarmDialog::slotOk()
{
  suspend();
}

void AlarmDialog::slotUser1()
{
  edit();
}

void AlarmDialog::slotUser2()
{
  dismissAll();
}

void AlarmDialog::slotUser3()
{
  dismissCurrent();
}

void AlarmDialog::dismissCurrent()
{
  ItemList selection = selectedItems();
  for ( ItemList::Iterator it = selection.begin(); it != selection.end(); ++it ) {
    if ( (*it)->itemBelow() )
      (*it)->itemBelow()->setSelected( true );
    else if ( (*it)->itemAbove() )
      (*it)->itemAbove()->setSelected( true );
    delete *it;
  }
  if ( activeCount() == 0 ) {
    writeLayout();
    accept();
  } else {
    updateButtons();
    showDetails();
  }
  emit reminderCount( activeCount() );
}

void AlarmDialog::dismissAll()
{
  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ) {
    AlarmListItem *item = static_cast<AlarmListItem*>( it.current() );
    if ( !item->isVisible() ) {
      ++it;
      continue;
    }
    mIncidenceListView->takeItem( item );
    delete item;
  }
  setTimer();
  writeLayout();
  accept();
  emit reminderCount( activeCount() );
}

void AlarmDialog::edit()
{
  ItemList selection = selectedItems();
  if ( selection.count() != 1 ) {
    return;
  }
  Incidence *incidence = mCalendar->incidence( selection.first()->mUid );
  if ( !incidence ) {
    return;
  }
  TQDate dt = selection.first()->mRemindAt.date();

  if ( incidence->isReadOnly() ) {
    KMessageBox::sorry(
      this,
      i18n( "\"%1\" is a read-only item so modifications are not possible." ).
      arg( cleanSummary( incidence->summary() ) ) );
    return;
  }

  if ( !ensureKorganizerRunning() ) {
    KMessageBox::error(
      this,
      i18n( "Could not start KOrganizer so editing is not possible." ) );
    return;
  }

  TQByteArray data;
  TQDataStream arg( data, IO_WriteOnly );
  arg << incidence->uid();
  arg << dt;
  //kdDebug(5890) << "editing incidence " << incidence->summary() << endl;
  if ( !kapp->dcopClient()->send( "korganizer", "KOrganizerIface",
                                  "editIncidence(TQString,TQDate)",
                                  data ) ) {
    KMessageBox::error(
      this,
      i18n( "An internal KOrganizer error occurred attempting to start the incidence editor" ) );
    return;
  }

  // get desktop # where korganizer (or kontact) runs
  TQByteArray replyData;
  TQCString object, replyType;
  object = kapp->dcopClient()->isApplicationRegistered( "kontact" ) ?
           "kontact-mainwindow#1" : "KOrganizer MainWindow";
  if (!kapp->dcopClient()->call( "korganizer", object,
                            "getWinID()", 0, replyType, replyData, true, -1 ) ) {
  }

  if ( replyType == "int" ) {
    int desktop, window;
    TQDataStream ds( replyData, IO_ReadOnly );
    ds >> window;
    desktop = KWin::windowInfo( window ).desktop();

    if ( KWin::currentDesktop() == desktop ) {
      KWin::iconifyWindow( winId(), false );
    } else {
      KWin::setCurrentDesktop( desktop );
    }
    KWin::activateWindow( KWin::transientFor( window ) );
  }
}

void AlarmDialog::suspend()
{
  if ( !isVisible() )
    return;

  int unit=1;
  switch (mSuspendUnit->currentItem()) {
    case 3: // weeks
      unit *=  7;
    case 2: // days
      unit *= 24;
    case 1: // hours
      unit *= 60;
    case 0: // minutes
      unit *= 60;
    default:
      break;
  }

  AlarmListItem *selitem = 0;
  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem * item = static_cast<AlarmListItem*>( it.current() );
    if ( item->isSelected() && item->isVisible() ) {
      item->setVisible( false );
      item->setSelected( false );
      item->mRemindAt = TQDateTime::currentDateTime().addSecs( unit * mSuspendSpin->value() );
      item->mNotified = false;
      selitem = item;
    }
  }
  if ( selitem ) {
    if ( selitem->itemBelow() ) {
      selitem->itemBelow()->setSelected( true );
    } else if ( selitem->itemAbove() ) {
      selitem->itemAbove()->setSelected( true );
    }
  }

  // save suspended alarms too so they can be restored on restart
  // kolab/issue4108
  slotSave();

  setTimer();
  if ( activeCount() == 0 ) {
    writeLayout();
    accept();
  } else {
    updateButtons();
    showDetails();
  }
  emit reminderCount( activeCount() );
}

void AlarmDialog::setTimer()
{
  int nextReminderAt = -1;
  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem * item = static_cast<AlarmListItem*>( it.current() );
    if ( item->mRemindAt > TQDateTime::currentDateTime() ) {
      int secs = TQDateTime::currentDateTime().secsTo( item->mRemindAt );
      nextReminderAt = nextReminderAt <= 0 ? secs : QMIN( nextReminderAt, secs );
    }
  }

  if ( nextReminderAt >= 0 ) {
    mSuspendTimer.stop();
    mSuspendTimer.start( 1000 * (nextReminderAt + 1), true );
  }
}

void AlarmDialog::show()
{
  mIncidenceListView->sort();

  // select the first item that hasn't already been notified
  mIncidenceListView->clearSelection();
  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem *item = static_cast<AlarmListItem*>( it.current() );
    if ( !item->mNotified ) {
      (*it)->setSelected( true );
      break;
    }
  }

  updateButtons();
  showDetails();

  // reset the default suspend time
  mSuspendSpin->setValue( defSuspendVal );
  mSuspendUnit->setCurrentItem( defSuspendUnit );

  KDialogBase::show();
  KWin::deIconifyWindow( winId(), false );
  KWin::setState( winId(), NET::KeepAbove | NET::DemandsAttention );
  KWin::setOnAllDesktops( winId(), true );
  KWin::activateWindow( winId() );
  raise();
  setActiveWindow();
  if ( isMinimized() ) {
    showNormal();
  }
  eventNotification();
}

void AlarmDialog::eventNotification()
{
  bool beeped = false, found = false;

  TQValueList<AlarmListItem*> list;
  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem *item = static_cast<AlarmListItem*>( it.current() );
    if ( !item->isVisible() || item->mNotified ) {
      continue;
    }
    Incidence *incidence = mCalendar->incidence( item->mUid );
    if ( !incidence ) {
      continue;
    }
    found = true;
    item->mNotified = true;
    Alarm::List alarms = incidence->alarms();
    Alarm::List::ConstIterator it;
    for ( it = alarms.begin(); it != alarms.end(); ++it ) {
      Alarm *alarm = *it;
      // FIXME: Check whether this should be done for all multiple alarms
      if (alarm->type() == Alarm::Procedure) {
        // FIXME: Add a message box asking whether the procedure should really be executed
        kdDebug(5890) << "Starting program: '" << alarm->programFile() << "'" << endl;
        KProcess proc;
        proc << TQFile::encodeName(alarm->programFile());
        proc.start(KProcess::DontCare);
      }
      else if (alarm->type() == Alarm::Audio) {
        beeped = true;
        KAudioPlayer::play(TQFile::encodeName(alarm->audioFile()));
      }
    }
  }

  if ( !beeped && found ) {
    KNotifyClient::beep();
  }
}

void AlarmDialog::wakeUp()
{
  bool activeReminders = false;
  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem *item = static_cast<AlarmListItem*>( it.current() );
    Incidence *incidence = mCalendar->incidence( item->mUid );
    if ( !incidence ) {
      delete item;
      continue;
    }

    if ( item->mRemindAt <= TQDateTime::currentDateTime() ) {
      if ( !item->isVisible() ) {
        item->setVisible( true );
        item->setSelected( false );
      }
      activeReminders = true;
    } else {
      item->setVisible( false );
    }
  }

  if ( activeReminders )
    show();
  setTimer();
  showDetails();
  emit reminderCount( activeCount() );
}

void AlarmDialog::slotSave()
{
  KConfig *config = kapp->config();
  KLockFile::Ptr lock = config->lockFile();
  if ( lock.data()->lock() != KLockFile::LockOK )
    return;

  config->setGroup( "General" );
  int numReminders = config->readNumEntry("Reminders", 0);

  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem *item = static_cast<AlarmListItem*>( it.current() );
    Incidence *incidence = mCalendar->incidence( item->mUid );
    if ( !incidence ) {
      continue;
    }
    config->setGroup( TQString("Incidence-%1").arg(numReminders + 1) );
    config->writeEntry( "UID", incidence->uid() );
    config->writeEntry( "RemindAt", item->mRemindAt );
    ++numReminders;
  }

  config->setGroup( "General" );
  config->writeEntry( "Reminders", numReminders );
  config->sync();
  lock.data()->unlock();
}

void AlarmDialog::closeEvent( TQCloseEvent * )
{
  slotSave();
  writeLayout();
  accept();
}

void AlarmDialog::updateButtons()
{
  ItemList selection = selectedItems();
  enableButton( User1, selection.count() == 1 ); // can only edit 1 at a time
  enableButton( User3, selection.count() > 0 );  // dismiss 1 or more
  enableButton( Ok, selection.count() > 0 );     // suspend 1 or more
}

TQValueList< AlarmListItem * > AlarmDialog::selectedItems() const
{
  TQValueList<AlarmListItem*> list;
  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    if ( it.current()->isSelected() )
      list.append( static_cast<AlarmListItem*>( it.current() ) );
  }
  return list;
}

int AlarmDialog::activeCount()
{
  int count = 0;
  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem * item = static_cast<AlarmListItem*>( it.current() );
    if ( item->isVisible() )
      ++count;
  }
  return count;
}

void AlarmDialog::suspendAll()
{
  mIncidenceListView->clearSelection();
  for ( TQListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    if ( it.current()->isVisible() )
      it.current()->setSelected( true );
  }
  suspend();
}

void AlarmDialog::showDetails()
{
  mDetailView->clearEvents( true );
  mDetailView->clear();
  AlarmListItem *item = static_cast<AlarmListItem*>( mIncidenceListView->selectedItems().first() );
  if ( !item || !item->isVisible() )
    return;

  Incidence *incidence = mCalendar->incidence( item->mUid );
  if ( !incidence ) {
    return;
  }

  if ( !item->mDisplayText.isEmpty() ) {
    TQString txt = "<qt><p><b>" + item->mDisplayText + "</b></p></qt>";
    mDetailView->addText( txt );
  }
  item->setText( 0, cleanSummary( incidence->summary() ) );
  mDetailView->appendIncidence( incidence, item->mRemindAt.date() );
}

bool AlarmDialog::ensureKorganizerRunning() const
{
  TQString error;
  TQCString dcopService;

  int result = KDCOPServiceStarter::self()->findServiceFor(
    "DCOP/Organizer", TQString::null, TQString::null, &error, &dcopService );

  if ( result == 0 ) {
    // OK, so korganizer (or kontact) is running. Now ensure the object we
    // want is available [that's not the case when kontact was already running,
    // but korganizer not loaded into it...]
    static const char* const dcopObjectId = "KOrganizerIface";
    TQCString dummy;
    if ( !kapp->dcopClient()->findObject(
           dcopService, dcopObjectId, "", TQByteArray(), dummy, dummy ) ) {
      DCOPRef ref( dcopService, dcopService ); // talk to KUniqueApplication or its kontact wrapper
      DCOPReply reply = ref.call( "load()" );
      if ( reply.isValid() && (bool)reply ) {
        Q_ASSERT( kapp->dcopClient()->findObject(
                    dcopService, dcopObjectId, "", TQByteArray(), dummy, dummy ) );
      } else {
        kdWarning() << "Error loading " << dcopService << endl;
      }
    }

    // We don't do anything with it we just need it to be running
    return true;

  } else {
    kdWarning() << "Couldn't start DCOP/Organizer: " << dcopService
                << " " << error << endl;
  }
  return false;
}

/** static */
TQDateTime AlarmDialog::triggerDateForIncidence( Incidence *incidence,
                                                const TQDateTime &reminderAt,
                                                TQString &displayStr )
{
  // Will be simplified in trunk, with roles.
  TQDateTime result;

  Alarm *alarm = incidence->alarms().first();

  if ( incidence->doesRecur() ) {
    result = incidence->recurrence()->getNextDateTime( reminderAt );
    displayStr = KGlobal::locale()->formatDateTime( result );
  }

  if ( incidence->type() == "Event" ) {
    if ( !result.isValid() ) {
      Event *event = static_cast<Event *>( incidence );
      result = alarm->hasStartOffset() ? event->dtStart() :
                                         event->dtEnd();
      displayStr = IncidenceFormatter::dateTimeToString( result, false, true );
    }
  } else if ( incidence->type() == "Todo" ) {
    if ( !result.isValid() ) {
      Todo *todo = static_cast<Todo *>( incidence );
      result = alarm->hasStartOffset() && todo->dtStart().isValid() ? todo->dtStart():
                                                                      todo->dtDue();
     displayStr = IncidenceFormatter::dateTimeToString( result, false, true );
    }
  }

  return result;
}

void AlarmDialog::slotCalendarChanged()
{
  Incidence::List incidences = mCalendar->incidences();
  for ( Incidence::List::ConstIterator it = incidences.begin();
        it != incidences.constEnd(); ++it ) {
    Incidence *incidence = *it;
    AlarmListItem *item = searchByUid( incidence->uid() );

    if ( item ) {
      TQString displayStr;
      const TQDateTime dateTime = triggerDateForIncidence( incidence,
                                                          item->mRemindAt,
                                                          displayStr );

      const TQString summary = cleanSummary( incidence->summary() );

      if ( displayStr != item->text( 1 ) || summary != item->text( 0 ) ) {
        item->setText( 1, displayStr );
        item->setText( 0, summary );
      }
    }
  }
}
