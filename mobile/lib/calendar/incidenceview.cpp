/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "incidenceview.h"
#include "calendarhelper.h"
#include "clockhelper.h"

#include "ui_dialogmobile.h"

#include <calendarsupport/utils.h>
#include <calendarsupport/kcalprefs.h>

#include <QMessageBox>
#include <QDateEdit>
#include <QTimeEdit>

#include <KDebug>
#include <KDialog>
#include <KLocalizedString>
#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/Calendar/ETMCalendar>

#include "declarativeeditors.h"

#include <qdeclarativeengine.h>
#include <qdeclarativecontext.h>

#include <incidenceeditor-ng/editorconfig.h>
#include <incidenceeditor-ng/incidencealarm.h>
#include <incidenceeditor-ng/incidenceattachment.h>
#include <incidenceeditor-ng/incidenceattendee.h>
#include <incidenceeditor-ng/incidencecategories.h>
#include <incidenceeditor-ng/incidencecompletionpriority.h>
#include <incidenceeditor-ng/incidencedatetime.h>
#include <incidenceeditor-ng/incidencedescription.h>
#include <incidenceeditor-ng/incidencewhatwhere.h>
#include <incidenceeditor-ng/incidencerecurrence.h>
#include <incidenceeditor-ng/incidencesecrecy.h>

#include <KMessageBox>

using namespace Akonadi;
using namespace IncidenceEditorNG;
using namespace KCalCore;
using namespace CalendarSupport;

IncidenceView::IncidenceView( QWidget* parent )
  : KDeclarativeFullScreenView( QLatin1String( "incidence-editor" ), parent )
  , mItemManager( new EditorItemManager( this ) )
  , mCollectionCombo( 0 )
  , mEditor( new CombinedIncidenceEditor( parent ) )
  , mEditorDateTime( 0 )
  , mIncidenceMore( 0 )
  , mIncidenceGeneral( 0 )
  , mDateWidget( 0 )
  , mTimeWidget( 0 )
  , mIncidenceAttendee( 0 )
{
  setAttribute(Qt::WA_DeleteOnClose);
  QDeclarativeContext *context = engine()->rootContext();
  context->setContextProperty( QLatin1String("_incidenceview"), this );
}

void IncidenceView::doDelayedInit()
{
  qmlRegisterType<DCollectionCombo>( "org.kde.incidenceeditors", 4, 5, "CollectionCombo" );
  qmlRegisterType<DIEGeneral>( "org.kde.incidenceeditors", 4, 5, "GeneralEditor" );
  qmlRegisterType<DIEMore>( "org.kde.incidenceeditors", 4, 5, "MoreEditor" );
  qmlRegisterType<CalendarHelper>( "CalendarHelper", 4, 5, "CalendarHelper" );
  qmlRegisterType<ClockHelper>( "ClockHelper", 4, 5, "ClockHelper" );

  connect( mItemManager, SIGNAL(itemSaveFinished(IncidenceEditorNG::EditorItemManager::SaveAction)),
           SLOT(slotSaveFinished(IncidenceEditorNG::EditorItemManager::SaveAction)) );
  connect( mItemManager, SIGNAL(itemSaveFailed(IncidenceEditorNG::EditorItemManager::SaveAction,QString)),
           SLOT(slotSaveFailed(IncidenceEditorNG::EditorItemManager::SaveAction,QString)) );
}

IncidenceView::~IncidenceView()
{
  delete mEditor;
}

void IncidenceView::load( const Akonadi::Item &item, const QDate &date )
{
  Q_ASSERT( item.hasPayload() ); // TODO: Fetch payload if there is no payload set.

  mItem = item;
  mItemManager->load( mItem );
  mActiveDate = date;

  if ( mCollectionCombo )
    mCollectionCombo->setDefaultCollection( mItem.parentCollection() );
}

void IncidenceView::setCollectionCombo( Akonadi::CollectionComboBox *combo )
{
  mCollectionCombo = combo;
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( mItem );
  mCollectionCombo->setMimeTypeFilter( QStringList() << incidence->mimeType() );
  mCollectionCombo->setAccessRightsFilter( Collection::CanCreateItem );

  if ( mDefaultCollection.isValid() )
    mCollectionCombo->setDefaultCollection( mDefaultCollection );
  else
    mCollectionCombo->setDefaultCollection( mItem.parentCollection() );
}

void IncidenceView::setGeneralEditor( MobileIncidenceGeneral *editorWidget )
{
  mIncidenceGeneral = editorWidget;

  Q_ASSERT( mItem.hasPayload<Incidence::Ptr>() );
  Incidence::Ptr incidencePtr = CalendarSupport::incidence( mItem );

  IncidenceEditorNG::IncidenceEditor *editor = new IncidenceEditorNG::IncidenceWhatWhere( editorWidget->mUi );
  mEditor->combine( editor );

  Q_ASSERT( mEditorDateTime == 0 );
  mEditorDateTime = new IncidenceEditorNG::IncidenceDateTime( editorWidget->mUi );
  mEditorDateTime->setActiveDate( mActiveDate );
  connect( mEditorDateTime, SIGNAL(startDateFocus(QObject*)), this, SLOT(showCalendar(QObject*)) );
  connect( mEditorDateTime, SIGNAL(endDateFocus(QObject*)), this, SLOT(showCalendar(QObject*)) );
  connect( mEditorDateTime, SIGNAL(startTimeFocus(QObject*)), this, SLOT(showClock(QObject*)) );
  connect( mEditorDateTime, SIGNAL(endTimeFocus(QObject*)), this, SLOT(showClock(QObject*)) );
  mEditor->combine( mEditorDateTime );

  editor = new IncidenceEditorNG::IncidenceCompletionPriority( editorWidget->mUi );
  mEditor->combine( editor );
  mEditor->load( incidencePtr );

  const QStringList allEmails = IncidenceEditorNG::EditorConfig::instance()->allEmails();
  const KCalCore::Attendee::Ptr me = incidencePtr->attendeeByMails( allEmails );

  if ( incidencePtr->attendeeCount() > 1 &&
       me && ( me->status() == KCalCore::Attendee::NeedsAction ||
               me->status() == KCalCore::Attendee::Tentative ||
               me->status() == KCalCore::Attendee::InProcess ) ) {
    editorWidget->mUi->mInvitationBar->show();
  } else {
    editorWidget->mUi->mInvitationBar->hide();
  }

  if ( mIncidenceMore != 0 ) { // IncidenceMore was set *before* general.
    initIncidenceMore();

    connect( editorWidget->mUi->mAcceptInvitationButton, SIGNAL(clicked()),
             mIncidenceAttendee, SLOT(acceptForMe()), Qt::UniqueConnection );
    connect( editorWidget->mUi->mDeclineInvitationButton, SIGNAL(clicked()),
             mIncidenceAttendee, SLOT(declineForMe()), Qt::UniqueConnection  );
  }

  connect( editorWidget->mUi->mAcceptInvitationButton, SIGNAL(clicked()),
           editorWidget->mUi->mInvitationBar, SLOT(hide()) );
  connect( editorWidget->mUi->mDeclineInvitationButton, SIGNAL(clicked()),
           editorWidget->mUi->mInvitationBar, SLOT(hide()) );
}

void IncidenceView::showCalendar( QObject *obj )
{
  /*### Workaround to force focus out, so
   the dialog doesn't reopen incorrectly */
  mIncidenceMore->setFocus();

  mDateWidget = qobject_cast<KDateComboBox*>( obj );
  if ( !mDateWidget )
    return;

  QDate date = mDateWidget->date();
  emit showCalendarWidget( date.day(), date.month(), date.year() );
}

void IncidenceView::setNewDate( int day, int month, int year )
{
  if ( mDateWidget == 0 )
    return;

  mDateWidget->setDate( QDate( year, month, day ) );
}

void IncidenceView::showClock( QObject *obj )
{
  /*### Workaround to force focus out, so
    the dialog doesn't reopen incorrectly */
  mIncidenceMore->setFocus();
  mTimeWidget = qobject_cast<KTimeComboBox*>( obj );
  if ( !mTimeWidget )
    return;

  QTime time = mTimeWidget->time();
  emit showClockWidget( time.hour(), time.minute() );
}

void IncidenceView::setNewTime( int hour, int minute )
{
  if ( mTimeWidget == 0 )
    return;

  mTimeWidget->setTime( QTime( hour, minute ) );
}

void IncidenceView::initIncidenceMore()
{
  Q_ASSERT( mItem.hasPayload<Incidence::Ptr>() );
  const Incidence::Ptr incidencePtr = CalendarSupport::incidence( mItem );

  IncidenceEditorNG::IncidenceEditor *editor = new IncidenceEditorNG::IncidenceCategories( mIncidenceMore->mUi );
  mEditor->combine( editor );

  editor = new IncidenceEditorNG::IncidenceDescription( mIncidenceMore->mUi );
  mEditor->combine( editor );

  mIncidenceAttendee = new IncidenceEditorNG::IncidenceAttendee( 0, mEditorDateTime, mIncidenceMore->mUi );
  mEditor->combine( mIncidenceAttendee );

  editor = new IncidenceEditorNG::IncidenceAlarm( mEditorDateTime, mIncidenceMore->mUi );
  mEditor->combine( editor );

  Q_ASSERT( mEditorDateTime != 0 );
  editor = new IncidenceEditorNG::IncidenceRecurrence( mEditorDateTime, mIncidenceMore->mUi );
  mEditor->combine( editor );

  editor = new IncidenceEditorNG::IncidenceSecrecy( mIncidenceMore->mUi );
  mEditor->combine( editor );

  editor = new IncidenceEditorNG::IncidenceAttachment( mIncidenceMore->mUi );
  mEditor->combine( editor );
  mEditor->load( incidencePtr );
}

void IncidenceView::setMoreEditor( MobileIncidenceMore *editorWidget )
{
  mIncidenceMore = editorWidget;

  if ( mEditorDateTime != 0 ) // IncidenceGeneral was not set yet.
    initIncidenceMore();

  if ( mIncidenceGeneral ) {
    connect( mIncidenceGeneral->mUi->mAcceptInvitationButton, SIGNAL(clicked()),
             mIncidenceAttendee, SLOT(acceptForMe()), Qt::UniqueConnection );
    connect( mIncidenceGeneral->mUi->mDeclineInvitationButton, SIGNAL(clicked()),
             mIncidenceAttendee, SLOT(declineForMe()), Qt::UniqueConnection  );
  }
}

void IncidenceView::setDefaultCollection( const Akonadi::Collection &collection )
{
  mDefaultCollection = collection;
}

void IncidenceView::setIsCounterProposal( bool isCounterProposal )
{
  mItemManager->setIsCounterProposal( isCounterProposal );
  //TODO_SERGIO
  //mInvitationDispatcher->setIsCounterProposal( isCounterProposal );
}

/// ItemEditorUi methods

bool IncidenceView::containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const
{
  return partIdentifiers.contains( QByteArray( "PLD:RFC822" ) );
}

bool IncidenceView::hasSupportedPayload( const Akonadi::Item &item ) const
{
  return item.hasPayload() && item.hasPayload<KCalCore::Incidence::Ptr>()
    && ( item.hasPayload<KCalCore::Event::Ptr>() || item.hasPayload<KCalCore::Todo::Ptr>() );
}

bool IncidenceView::isDirty() const
{
  return mEditor->isDirty();
}

bool IncidenceView::isValid() const
{
  return mEditor->isValid();
}

void IncidenceView::load( const Akonadi::Item &item )
{
  Q_ASSERT( hasSupportedPayload( item ) );

  mItem = item;
  mEditor->load( CalendarSupport::incidence( item ) );
}

Akonadi::Item IncidenceView::save( const Akonadi::Item &item )
{
  if ( !hasSupportedPayload( mItem ) ) {
    kWarning() << "Item id=" << mItem.id() << "remoteId=" << mItem.remoteId()
               << "mime=" << mItem.mimeType() << "does not have a supported MIME type";
    return item;
  }

  KCalCore::Incidence::Ptr incidenceInEditor = mEditor->incidence<KCalCore::Incidence>();
  KCalCore::Incidence::Ptr incidence( incidenceInEditor->clone() );

  mEditor->save( incidence );

  // Mark the incidence as changed
  if ( mItem.isValid() )
    incidence->setRevision( incidence->revision() + 1 );

  Akonadi::Item result = item;
  result.setPayload<KCalCore::Incidence::Ptr>( incidence );
  result.setMimeType( mItem.mimeType() );
  return result;
}

Akonadi::Collection IncidenceView::selectedCollection() const
{
  return mCollectionCombo->currentCollection();
}

void IncidenceView::reject( RejectReason /*reason*/, const QString &errorMessage )
{
  kDebug() << "Rejecting:" << errorMessage;
  deleteLater();
}

/// IncidenceView slots

void IncidenceView::save()
{
  mEditor->focusInvalidField();
  if ( mEditor->isValid() ) {
    if ( mCollectionCombo->currentCollection().isValid() ) {
      mItemManager->save();
    } else {
      KMessageBox::sorry( this, i18n( "Please select an account" ) );
      kDebug() << "No collection selected";
    }
  } else {
    kDebug() << "Editor content isn't valid because: " << mEditor->lastErrorString();
  }
}

void IncidenceView::slotSaveFinished( IncidenceEditorNG::EditorItemManager::SaveAction action )
{
  Q_UNUSED( action );
  deleteLater();
}

void IncidenceView::slotSaveFailed( IncidenceEditorNG::EditorItemManager::SaveAction action,
                                    const QString &message )
{
  Q_UNUSED( action );

  QPointer<QMessageBox> dlg = new QMessageBox; //krazy:exclude=qclasses
  dlg->setIcon( QMessageBox::Warning );
  dlg->setWindowTitle( i18n( "Saving the event failed." ) );
  dlg->setInformativeText( i18n( "Reason:\n\n" ) + message );
  dlg->addButton( i18n( "OK" ), QMessageBox::AcceptRole );
  dlg->exec();
  delete dlg;
}

void IncidenceView::cancel()
{
  deleteLater();
}

