/*
    This file is part of KOrganizer.

    Copyright (c) 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
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

#include <tqtooltip.h>
#include <tqframe.h>
#include <tqpixmap.h>
#include <tqlayout.h>
#include <tqwidgetstack.h>
#include <tqwhatsthis.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/incidenceformatter.h>
#include <libkcal/calendarlocal.h>

#include "koprefs.h"
#include "koeditorgeneralevent.h"
#include "koeditorrecurrence.h"
#include "koeditordetails.h"
#include "koeditorfreebusy.h"
#include "kogroupware.h"
#include "kodialogmanager.h"
#include "incidencechanger.h"

#include "koeventeditor.h"

KOEventEditor::KOEventEditor( Calendar *calendar, TQWidget *parent )
  : KOIncidenceEditor( TQString::null, calendar, parent ),
    mEvent( 0 ), mCalendar( 0 ), mGeneral( 0 ), mRecurrence( 0 ), mFreeBusy( 0 )
{
}

KOEventEditor::~KOEventEditor()
{
  if ( !mIsCounter )
    emit dialogClose( mEvent );
}

void KOEventEditor::init()
{
  setupGeneral();
  setupRecurrence();
  setupFreeBusy();
  setupDesignerTabs( "event" );

  // Propagate date time settings to recurrence tab
  connect( mGeneral, TQT_SIGNAL( dateTimesChanged( const TQDateTime &, const TQDateTime & ) ),
           mRecurrence, TQT_SLOT( setDateTimes( const TQDateTime &, const TQDateTime &) ) );
  connect( mGeneral, TQT_SIGNAL( dateTimeStrChanged( const TQString & ) ),
           mRecurrence, TQT_SLOT( setDateTimeStr( const TQString & ) ) );
  connect( mFreeBusy, TQT_SIGNAL( dateTimesChanged( const TQDateTime &, const TQDateTime & ) ),
           mRecurrence, TQT_SLOT( setDateTimes( const TQDateTime &, const TQDateTime & ) ) );

  // Propagate date time settings to gantt tab and back
  connect( mGeneral, TQT_SIGNAL( dateTimesChanged( const TQDateTime &, const TQDateTime & ) ),
           mFreeBusy, TQT_SLOT( slotUpdateGanttView( const TQDateTime &, const TQDateTime & ) ) );
  connect( mFreeBusy, TQT_SIGNAL( dateTimesChanged( const TQDateTime &, const TQDateTime & ) ),
           mGeneral, TQT_SLOT( setDateTimes( const TQDateTime &, const TQDateTime & ) ) );

  connect( mGeneral, TQT_SIGNAL( focusReceivedSignal() ),
           TQT_SIGNAL( focusReceivedSignal() ) );

  connect( mGeneral, TQT_SIGNAL( openCategoryDialog() ),
           TQT_SIGNAL( editCategories() ) );
  connect( this, TQT_SIGNAL( updateCategoryConfig() ),
           mGeneral, TQT_SIGNAL( updateCategoryConfig() ) );

  connect( mFreeBusy, TQT_SIGNAL(updateAttendeeSummary(int)),
           mGeneral, TQT_SLOT(updateAttendeeSummary(int)) );

  connect( mGeneral, TQT_SIGNAL(editRecurrence()),
           mRecurrenceDialog, TQT_SLOT(show()) );
  connect( mRecurrenceDialog, TQT_SIGNAL(okClicked()),
           TQT_SLOT(updateRecurrenceSummary()) );

  connect( mGeneral, TQT_SIGNAL(acceptInvitation()),
           mFreeBusy, TQT_SLOT(acceptForMe()) );
  connect( mGeneral, TQT_SIGNAL(declineInvitation()),
           mFreeBusy, TQT_SLOT(declineForMe()) );
}

void KOEventEditor::reload()
{
  kdDebug(5850) << "KOEventEditor::reload()" << endl;

  if ( mEvent ) {
    readEvent( mEvent, mCalendar, TQDate() );
  }
}

void KOEventEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralEvent( this );

  if( KOPrefs::instance()->mCompactDialogs ) {
    TQFrame *topFrame = addPage(i18n("General"));
    TQWhatsThis::add( topFrame,
                     i18n("The General tab allows you to set the most common "
                          "options for the event.") );

    TQBoxLayout *topLayout = new TQVBoxLayout(topFrame);
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader( topFrame, topLayout );
    mGeneral->initTime(topFrame,topLayout);

    topLayout->addStretch( 1 );

    TQFrame *topFrame2 = addPage(i18n("Details"));

    TQBoxLayout *topLayout2 = new TQVBoxLayout(topFrame2);
    topLayout2->setSpacing(spacingHint());

    mGeneral->initClass(topFrame2,topLayout2);
    mGeneral->initSecrecy( topFrame2, topLayout2 );
    mGeneral->initDescription(topFrame2,topLayout2);
  } else {
    TQFrame *topFrame = addPage(i18n("&General"));
    TQWhatsThis::add( topFrame,
                     i18n("The General tab allows you to set the most common "
                          "options for the event.") );

    TQBoxLayout *topLayout = new TQVBoxLayout(topFrame);
    topLayout->setSpacing(spacingHint());

    mGeneral->initInvitationBar( topFrame, topLayout );
    mGeneral->initHeader( topFrame, topLayout );
    mGeneral->initTime(topFrame,topLayout);
    mGeneral->initDescription(topFrame,topLayout);
    mGeneral->initAttachments(topFrame,topLayout);
    connect( mGeneral, TQT_SIGNAL( openURL( const KURL& ) ),
             this, TQT_SLOT( openURL( const KURL& ) ) );
    connect( this, TQT_SIGNAL( signalAddAttachments( const TQStringList&, const TQStringList&, bool ) ),
             mGeneral, TQT_SLOT( addAttachments( const TQStringList&, const TQStringList&, bool ) ) );
  }

  mGeneral->finishSetup();
}

void KOEventEditor::modified()
{
  // Play dumb, just reload the event. This dialog has become so complicated
  // that there is no point in trying to be smart here...
  reload();
}

void KOEventEditor::setupRecurrence()
{
  mRecurrenceDialog = new KOEditorRecurrenceDialog( this );
  mRecurrenceDialog->hide();
  mRecurrence = mRecurrenceDialog->editor();
}

void KOEventEditor::setupFreeBusy()
{
  TQFrame *freeBusyPage = addPage( i18n("&Attendees") );
  TQWhatsThis::add( freeBusyPage,
        i18n("The Free/Busy tab allows you to see whether "
       "other attendees are free or busy during your event.") );

  TQBoxLayout *topLayout = new TQVBoxLayout( freeBusyPage );

  mAttendeeEditor = mFreeBusy = new KOEditorFreeBusy( spacingHint(), freeBusyPage );
  topLayout->addWidget( mFreeBusy );
}

void KOEventEditor::editIncidence( Incidence *incidence,
                                   const TQDate &date,
                                   Calendar *calendar )
{
  Event*event = dynamic_cast<Event*>(incidence);
  if ( event ) {
    init();

    mEvent = event;
    mCalendar = calendar;

    const TQDate &dt = mRecurIncidence && date.isValid() ? date : incidence->dtStart().date();
    readEvent( mEvent, mCalendar, dt );
  }

  setCaption( i18n("Edit Event") );
}

void KOEventEditor::newEvent()
{
  init();
  mEvent = 0;
  loadDefaults();
  setCaption( i18n("New Event") );
}

void KOEventEditor::setDates( const TQDateTime &from, const TQDateTime &to, bool allDay )
{
  mGeneral->setDefaults( from, to, allDay );
  mRecurrence->setDefaults( from, to, allDay );
  if( mFreeBusy ) {
    if ( allDay )
      mFreeBusy->setDateTimes( from, to.addDays( 1 ) );
    else
      mFreeBusy->setDateTimes( from, to );
  }
}

void KOEventEditor::setTexts( const TQString &summary, const TQString &description )
{
  if ( description.isEmpty() && summary.contains("\n") ) {
    mGeneral->setDescription( summary );
    int pos = summary.find( "\n" );
    mGeneral->setSummary( summary.left( pos ) );
  } else {
    mGeneral->setSummary( summary );
    mGeneral->setDescription( description );
  }
}

void KOEventEditor::loadDefaults()
{
  TQDateTime from( TQDate::currentDate(), KOPrefs::instance()->mStartTime.time() );
  int addSecs = ( KOPrefs::instance()->mDefaultDuration.time().hour()*3600 ) +
                ( KOPrefs::instance()->mDefaultDuration.time().minute()*60 );
  TQDateTime to( from.addSecs( addSecs ) );

  setDates( from, to, false );
}

bool KOEventEditor::processInput()
{
  kdDebug(5850) << "KOEventEditor::processInput(); event is " << mEvent << endl;

  if ( !validateInput() || !mChanger ) {
    kdDebug(5850) << " mChanger is " << mChanger << endl;
    return false;
  }

  TQGuardedPtr<KOEditorFreeBusy> freeBusy( mFreeBusy );

  if ( mEvent ) {
    bool rc = true;
    Event *oldEvent = mEvent->clone();
    Event *event = mEvent->clone();

    kdDebug(5850) << "KOEventEditor::processInput() write event." << endl;
    writeEvent( event );
    kdDebug(5850) << "KOEventEditor::processInput() event written." << endl;

    if( *event == *mEvent ) {
      // Don't do anything
      kdDebug(5850) << "Event not changed" << endl;
      if ( mIsCounter ) {
        KMessageBox::information( this, i18n("You didn't change the event, thus no counter proposal has been sent to the organizer."), i18n("No changes") );
      }
    } else {
      kdDebug(5850) << "Event changed" << endl;
      //IncidenceChanger::assignIncidence( mEvent, event );
      writeEvent( mEvent );
      if ( mIsCounter ) {
        KOGroupware::instance()->sendCounterProposal( mCalendar, oldEvent, mEvent );
        // add dummy event at the position of the counter proposal
        Event *event = mEvent->clone();
        event->clearAttendees();
        event->setSummary( i18n("My counter proposal for: %1").arg( mEvent->summary() ) );
        mChanger->addIncidence( event, mResource, mSubResource, this );
      } else {
        if ( mRecurIncidence && mRecurIncidenceAfterDissoc ) {
          mChanger->addIncidence( mEvent, mResource, mSubResource, this );

          mChanger->changeIncidence( mRecurIncidence, mRecurIncidenceAfterDissoc,
                                     KOGlobals::RECURRENCE_MODIFIED_ALL_FUTURE, this );

        } else {
          mChanger->changeIncidence( oldEvent, mEvent, KOGlobals::NOTHING_MODIFIED, this );
        }
      }
    }
    delete event;
    delete oldEvent;
    return rc;
  } else {
    mEvent = new Event;
    mEvent->setOrganizer( Person( KOPrefs::instance()->fullName(),
                          KOPrefs::instance()->email() ) );
    writeEvent( mEvent );
    // NOTE: triggered by addIncidence, the kolab resource might open a non-modal dialog (parent is not available in the resource) to select a resource folder. Thus the user can close this dialog before addIncidence() returns.
    if ( !mChanger->addIncidence( mEvent, mResource, mSubResource, this ) ) {
      delete mEvent;
      mEvent = 0;
      return false;
    }
  }
  // if "this" was deleted, freeBusy is 0 (being a guardedptr)
  if ( freeBusy ) {
    freeBusy->cancelReload();
  }

  return true;
}

void KOEventEditor::processCancel()
{
  kdDebug(5850) << "KOEventEditor::processCancel()" << endl;

  if ( mFreeBusy ) mFreeBusy->cancelReload();

  if ( mRecurIncidence && mRecurIncidenceAfterDissoc ) {
    *mRecurIncidenceAfterDissoc = *mRecurIncidence;
  }

}

void KOEventEditor::deleteEvent()
{
  kdDebug(5850) << "Delete event" << endl;

  if ( mEvent )
    emit deleteIncidenceSignal( mEvent );
  emit dialogClose( mEvent );
  reject();
}

void KOEventEditor::readEvent( Event *event, Calendar *calendar, const TQDate &date, bool tmpl )
{
  mGeneral->readEvent( event, calendar, date, tmpl );
  mRecurrence->readIncidence( event );

  if ( mFreeBusy ) {
    mFreeBusy->readEvent( event );
    mFreeBusy->triggerReload();
  }

  createEmbeddedURLPages( event );
  readDesignerFields( event );

  if ( mIsCounter )
    mGeneral->invitationBar()->hide();
}

void KOEventEditor::writeEvent( Event *event )
{
  mGeneral->writeEvent( event );
  if ( mFreeBusy )
    mFreeBusy->writeEvent( event );

  cancelRemovedAttendees( event );

  mRecurrence->writeIncidence( event );

  writeDesignerFields( event );
}

bool KOEventEditor::validateInput()
{
  if ( !mGeneral->validateInput() ||
       !mDetails->validateInput() ||
       !mRecurrence->validateInput() ) {
    kdDebug(5850) << "ValidateInput returns false" << endl;
    return false;
  } else {
    return true;
  }
}

int KOEventEditor::msgItemDelete()
{
  return KMessageBox::warningContinueCancel(this,
      i18n("This item will be permanently deleted."),
      i18n("KOrganizer Confirmation"),KGuiItem(i18n("Delete"),"editdelete"));
}

void KOEventEditor::loadTemplate( /*const*/ CalendarLocal& cal )
{
  const Event::List events = cal.events();
  if ( events.count() == 0 ) {
    KMessageBox::error( this,
        i18n("Template does not contain a valid event.") );
  } else {
    kdDebug(5850) << "KOEventEditor::slotLoadTemplate(): readTemplate" << endl;
    readEvent( events.first(), 0, TQDate(), true );
  }
}

TQStringList& KOEventEditor::templates() const
{
  return KOPrefs::instance()->mEventTemplates;
}

void KOEventEditor::slotSaveTemplate( const TQString &templateName )
{
  kdDebug(5006) << "SlotSaveTemplate" << endl;
  Event *event = new Event;
  writeEvent( event );
  saveAsTemplate( event, templateName );
}

TQObject *KOEventEditor::typeAheadReceiver() const
{
  return mGeneral->typeAheadReceiver();
}

void KOEventEditor::updateRecurrenceSummary()
{
  Event *ev = new Event();
  writeEvent( ev );
  mGeneral->updateRecurrenceSummary( ev );
  delete ev;
}

void KOEventEditor::selectInvitationCounterProposal(bool enable)
{
  KOIncidenceEditor::selectInvitationCounterProposal( enable );
  if ( enable )
    mGeneral->invitationBar()->hide();
}

#include "koeventeditor.moc"
