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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "eventeditor.h"
#include "editorconfig.h"
#ifdef HAVE_QT3SUPPORT
#include "editordetails.h"
#include "editorfreebusy.h"
#endif
#include "editorgeneralevent.h"
#include "editorrecurrence.h"
#include "incidencegeneraleditor.h"

#include <akonadi/kcal/utils.h> //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/incidencechanger.h>
#include <akonadi/kcal/groupware.h>

#include <Akonadi/CollectionComboBox>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include <KLocale>
#include <KMessageBox>

#include <QFrame>
#include <QTabWidget>
#include <QVBoxLayout>

using namespace KCal;
using namespace Akonadi;
using namespace IncidenceEditors;

EventEditor::EventEditor( QWidget *parent )
  : IncidenceEditor( QString(),
                     QStringList() << Akonadi::IncidenceMimeTypeVisitor::eventMimeType(),
                     parent ),
    mGeneral( 0 ), mRecurrence( 0 ), mFreeBusy( 0 )
{
  mInitialEvent = Event::Ptr( new Event );
  mInitialEventItem.setPayload(mInitialEvent);
}

EventEditor::~EventEditor()
{
  if ( !mIsCounter ) {
    emit dialogClose( mIncidence );
  }
}

bool EventEditor::incidenceModified()
{
  Event::Ptr oldEvent;
  if ( Akonadi::hasEvent( mIncidence ) ) { // modification
    oldEvent = Akonadi::event(mIncidence);
  } else { // new one
    oldEvent = mInitialEvent;
  }

  Event::Ptr newEvent( oldEvent->clone() );
  Akonadi::Item newEventItem;
  newEventItem.setPayload(newEvent);
  fillEvent( newEventItem );

  const bool modified = !( *newEvent == *oldEvent );
  return modified;
}

void EventEditor::init()
{
  setupGeneral();
  setupRecurrence();
  setupFreeBusy();
  setupDesignerTabs( "event" );

  // Propagate date time settings to recurrence tab
  connect( mGeneral, SIGNAL(dateTimesChanged(const QDateTime&,const QDateTime& )),
           mRecurrence, SLOT(setDateTimes(const QDateTime&,const QDateTime&)) );
  connect( mGeneral, SIGNAL(dateTimeStrChanged(const QString&)),
           mRecurrence, SLOT(setDateTimeStr(const QString&)) );
#ifdef HAVE_QT3SUPPORT
  connect( mFreeBusy, SIGNAL(dateTimesChanged(const QDateTime&,const QDateTime&)),
           mRecurrence, SLOT(setDateTimes(const QDateTime&,const QDateTime&)) );
#endif

  // Propagate date time settings to gantt tab and back
#ifdef HAVE_QT3SUPPORT
  connect( mGeneral, SIGNAL(dateTimesChanged(const QDateTime&,const QDateTime&)),
           mFreeBusy, SLOT(slotUpdateGanttView(const QDateTime&,const QDateTime&)) );
  connect( mFreeBusy, SIGNAL(dateTimesChanged(const QDateTime&,const QDateTime&)),
           mGeneral, SLOT(setDateTimes(const QDateTime&,const QDateTime&)) );
#endif

  connect( mGeneral, SIGNAL(focusReceivedSignal()),
           SIGNAL(focusReceivedSignal()) );

  connect( mGeneral, SIGNAL(openCategoryDialog()),
           SIGNAL(editCategories()) );
  connect( this, SIGNAL(updateCategoryConfig()),
           mGeneral, SIGNAL(updateCategoryConfig()) );

#ifdef HAVE_QT3SUPPORT
  connect( mFreeBusy, SIGNAL(updateAttendeeSummary(int)),
           mGeneral, SLOT(updateAttendeeSummary(int)) );
#endif

  connect( mGeneral, SIGNAL(editRecurrence()),
           mRecurrenceDialog, SLOT(show()) );
  connect( mRecurrenceDialog, SIGNAL(okClicked()),
           SLOT(updateRecurrenceSummary()) );

#ifdef HAVE_QT3SUPPORT
  connect( mGeneral, SIGNAL(acceptInvitation()),
           mFreeBusy, SLOT(acceptForMe()) );
  connect( mGeneral, SIGNAL(declineInvitation()),
           mFreeBusy, SLOT(declineForMe()) );
#endif

  updateRecurrenceSummary();
}

void EventEditor::setupGeneral()
{
  mGeneral = new EditorGeneralEvent( this );

  QFrame *topFrame = new QFrame();
  mTabWidget->addTab( topFrame, i18nc( "@title:tab general event settings", "&General" ) );
//   mTabWidget->addTab( new EventGeneralEditor( this ), "&New General" );
  topFrame->setWhatsThis( i18nc( "@info:whatsthis",
                                 "The General tab allows you to set the most "
                                 "common options for the event." ) );

  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  mGeneral->initInvitationBar( topFrame, topLayout );
  mGeneral->initHeader( topFrame, topLayout );
  mGeneral->initTime( topFrame, topLayout );
  mGeneral->initDescription( topFrame, topLayout );
  mGeneral->initAttachments( topFrame, topLayout );
  connect( mGeneral, SIGNAL(openURL(const KUrl&)),
           this, SLOT(openURL(const KUrl&)) );
  connect( this, SIGNAL(signalAddAttachments(const QStringList&,const QStringList&,bool)),
           mGeneral, SLOT(addAttachments(const QStringList&,const QStringList&,bool)) );

  mGeneral->finishSetup();
}

void EventEditor::modified()
{
  // Play dumb, just reload the event. This dialog has become so complicated
  // that there is no point in trying to be smart here...
  readIncidence( mIncidence, QDate(), true );
}

void EventEditor::setupRecurrence()
{
  mRecurrenceDialog = new EditorRecurrenceDialog( this );
  mRecurrenceDialog->hide();
  mRecurrence = mRecurrenceDialog->editor();
}

void EventEditor::setupFreeBusy()
{
  QFrame *freeBusyPage = new QFrame();
  mTabWidget->addTab( freeBusyPage, i18nc( "@title:tab", "&Attendees" ) );
  freeBusyPage->setWhatsThis( i18nc( "@info:whatsthis",
                                     "The Free/Busy tab allows you to see "
                                     "whether other attendees are free or busy "
                                     "during your event." ) );

  QVBoxLayout *topLayout = new QVBoxLayout( freeBusyPage );
  topLayout->setMargin(0);

#ifdef HAVE_QT3SUPPORT
  mAttendeeEditor = mFreeBusy = new EditorFreeBusy( spacingHint(), freeBusyPage );
  topLayout->addWidget( mFreeBusy );
#endif
}

void EventEditor::newEvent()
{
  init();
  mIncidence = Item();
  loadDefaults();
  setCaption( i18nc( "@title:window", "New Event" ) );
}

void EventEditor::setDates( const QDateTime &from, const QDateTime &to, bool allDay )
{
  mGeneral->setDefaults( from, to, allDay );
  mRecurrence->setDefaults( from, to, allDay );

  if ( mFreeBusy ) {
#ifdef HAVE_QT3SUPPORT
    if ( allDay ) {
      mFreeBusy->setDateTimes( from, to.addDays( 1 ) );
    } else {
      mFreeBusy->setDateTimes( from, to );
    }
#endif
  }
}

void EventEditor::setTexts( const QString &summary, const QString &description,
                              bool richDescription )
{
  if ( description.isEmpty() && summary.contains( "\n" ) ) {
    mGeneral->setDescription( summary, richDescription );
    int pos = summary.indexOf( "\n" );
    mGeneral->setSummary( summary.left( pos ) );
  } else {
    mGeneral->setSummary( summary );
    mGeneral->setDescription( description, richDescription );
  }
}

void EventEditor::loadDefaults()
{
  QDateTime from( QDate::currentDate(), EditorConfig::instance()->startTime().time() );
  int addSecs = ( EditorConfig::instance()->defaultDuration().time().hour() * 3600 ) +
                ( EditorConfig::instance()->defaultDuration().time().minute() * 60 );
  QDateTime to( from.addSecs( addSecs ) );

  setDates( from, to, false );
}

bool EventEditor::processInput()
{
  kDebug();
  if ( !validateInput() || !mChanger ) {
    return false;
  }

#ifdef HAVE_QT3SUPPORT
  QPointer<EditorFreeBusy> freeBusy( mFreeBusy );
#endif

  if ( Akonadi::hasEvent( mIncidence ) ) {
    Event::Ptr ev = Akonadi::event( mIncidence );
    bool rc = true;
    Event::Ptr oldEvent( ev->clone() );
    Event::Ptr event( ev->clone() );

    Akonadi::Item eventItem;
    eventItem.setPayload(event);
    fillEvent( eventItem );

    if ( *event == *oldEvent ) {
      // Don't do anything
      if ( mIsCounter ) {
        KMessageBox::information(
          this,
          i18nc( "@info",
                 "You did not modify the event so no counter proposal has "
                 "been sent to the organizer." ),
          i18nc( "@title:window", "No Changes" ) );
      }
    } else {
      if ( mIsCounter ) {
        Q_ASSERT( mIncidence.hasPayload<KCal::Event::Ptr>() );
        Akonadi::Groupware::instance()->sendCounterProposal( oldEvent.get(), event.get() );

        // add dummy event at the position of the counter proposal
        Event::Ptr event2( event->clone() );
        event2->clearAttendees();
        event2->setSummary(
          i18nc( "@item",
                 "My counter proposal for: %1", ev->summary() ) );

        Akonadi::Collection col = mCalSelector->currentCollection();
        rc = mChanger->addIncidence( event2, col, this );
      } else {
        if ( mChanger->beginChange( mIncidence ) ) {
          ev->startUpdates(); //merge multiple mIncidence->updated() calls into one
          fillEvent(mIncidence);
          rc = mChanger->changeIncidence( oldEvent,
                                          mIncidence,
                                          Akonadi::IncidenceChanger::NOTHING_MODIFIED,
                                          this );
          ev->endUpdates();
          mChanger->endChange( mIncidence );
        } else {
          return false;
        }
      }
    }
    return rc;
  } else {
    //PENDING(AKONADI_PORT) review mIncidence will differ from newly created item
    Event::Ptr newEvent( new Event );
    newEvent->setOrganizer( Person( EditorConfig::instance()->fullName(),
                            EditorConfig::instance()->email() ) );
    mIncidence.setPayload( newEvent );
    fillEvent( mIncidence );
    Akonadi::Collection col = mCalSelector->currentCollection();
    if ( !mChanger->addIncidence( newEvent, col, this ) ) {
      mIncidence = Item();
      return false;
    }
  }

  // if "this" was deleted, freeBusy is 0 (being a guardedptr)
#ifdef HAVE_QT3SUPPORT
  if ( freeBusy ) {
    freeBusy->cancelReload();
  }
#endif

  return true;
}

void EventEditor::processCancel()
{
  if ( mFreeBusy ) {
#ifdef HAVE_QT3SUPPORT
    mFreeBusy->cancelReload();
#endif
  }
}

void EventEditor::deleteEvent()
{
  if ( Akonadi::hasEvent( mIncidence ) ) {
    emit deleteIncidenceSignal( mIncidence );
  }

  emit dialogClose( mIncidence );
  reject();
}

bool EventEditor::read( const Item &eventItem, const QDate &date, bool tmpl )
{
  if ( !Akonadi::hasEvent( eventItem ) ) {
    return false;
  }

  const Event::Ptr event = Akonadi::event( eventItem );
  mGeneral->readEvent( event.get(), date, tmpl );
  mRecurrence->readIncidence( event.get() );

  if ( mFreeBusy ) {
#ifdef HAVE_QT3SUPPORT
    mFreeBusy->readIncidence( event.get() );
    mFreeBusy->triggerReload();
#endif
  }

  createEmbeddedURLPages( event.get() );
  readDesignerFields( eventItem );

  if ( mIsCounter ) {
    mGeneral->invitationBar()->hide();
  }
  return true;
}

void EventEditor::fillEvent( const Akonadi::Item &item )
{
  KCal::Event::Ptr event = Akonadi::event(item);
  mGeneral->fillEvent( event.get() );
  if ( mFreeBusy ) {
#ifdef HAVE_QT3SUPPORT
    mFreeBusy->fillIncidence( event.get() );
#endif
  }
  cancelRemovedAttendees( item );
  mRecurrence->fillIncidence( event.get() );
  writeDesignerFields( event.get() );
}

bool EventEditor::validateInput()
{
  if ( !mGeneral->validateInput() ) {
    return false;
  }
#ifdef HAVE_QT3SUPPORT
  if ( !mDetails->validateInput() ) {
    return false;
  }
#endif
  if ( !mRecurrence->validateInput() ) {
    return false;
  }
  return true;
}

QObject *EventEditor::typeAheadReceiver() const
{
  return mGeneral->typeAheadReceiver();
}

void EventEditor::updateRecurrenceSummary()
{
  Event::Ptr ev( new Event );

  Akonadi::Item evItem;
  evItem.setPayload(ev);
  fillEvent( evItem );

  mGeneral->updateRecurrenceSummary( ev.get() );
}

void EventEditor::selectInvitationCounterProposal( bool enable )
{
  IncidenceEditor::selectInvitationCounterProposal( enable );
  if ( enable ) {
    mGeneral->invitationBar()->hide();
  }
}

void EventEditor::show()
{

  fillEvent( mInitialEventItem );
  IncidenceEditor::show();
}

#include "eventeditor.moc"
