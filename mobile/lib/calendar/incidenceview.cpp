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

#include <calendarsupport/utils.h>

#include <QtGui/QMessageBox>

#include <KDebug>
#include <KDialog>
#include <KLocalizedString>

#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>


#include "declarativeeditors.h"

#include <qdeclarativeengine.h>
#include <qdeclarativecontext.h>

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


using namespace Akonadi;
using namespace IncidenceEditorsNG;
using namespace KCalCore;
using namespace CalendarSupport;

IncidenceView::IncidenceView( QWidget* parent )
  : KDeclarativeFullScreenView( QLatin1String( "incidence-editor" ), parent )
  , mItemManager( new EditorItemManager( this ) )
  , mCollectionCombo( 0 )
  , mEditor( new CombinedIncidenceEditor( parent ) )
  , mEditorDateTime( 0 )
  , mIncidenceMore( 0 )
{
  setAttribute(Qt::WA_DeleteOnClose);
}

void IncidenceView::delayedInit()
{
  KDeclarativeFullScreenView::delayedInit();
  qmlRegisterType<DCollectionCombo>( "org.kde.incidenceeditors", 4, 5, "CollectionCombo" );
  qmlRegisterType<DIEGeneral>( "org.kde.incidenceeditors", 4, 5, "GeneralEditor" );
  qmlRegisterType<DIEMore>( "org.kde.incidenceeditors", 4, 5, "MoreEditor" );
  qmlRegisterType<CalendarHelper>( "CalendarHelper", 4, 5, "CalendarHelper" );

  connect( mItemManager, SIGNAL(itemSaveFinished(CalendarSupport::EditorItemManager::SaveAction)),
           SLOT(slotSaveFinished(CalendarSupport::EditorItemManager::SaveAction) ) );
  connect( mItemManager, SIGNAL(itemSaveFailed(CalendarSupport::EditorItemManager::SaveAction, QString)),
           SLOT(slotSaveFailed(CalendarSupport::EditorItemManager::SaveAction, QString) ) );
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
  mCollectionCombo->setMimeTypeFilter( QStringList() << KCalCore::Event::eventMimeType() );
  mCollectionCombo->setAccessRightsFilter( Collection::CanCreateItem );
  mCollectionCombo->setDefaultCollection( mItem.parentCollection() );
}

void IncidenceView::setGeneralEditor( MobileIncidenceGeneral *editorWidget )
{
  Q_ASSERT( mItem.hasPayload<Incidence::Ptr>() );
  Incidence::Ptr incidencePtr = CalendarSupport::incidence( mItem );

  IncidenceEditorsNG::IncidenceEditor *editor = new IncidenceEditorsNG::IncidenceWhatWhere( editorWidget->mUi );
  mEditor->combine( editor );

  Q_ASSERT( mEditorDateTime == 0 );
  mEditorDateTime = new IncidenceEditorsNG::IncidenceDateTime( editorWidget->mUi );
  mEditorDateTime->setActiveDate( mActiveDate );
  mEditor->combine( mEditorDateTime );

  editor = new IncidenceEditorsNG::IncidenceCompletionPriority( editorWidget->mUi );
  mEditor->combine( editor );
  mEditor->load( incidencePtr );

  if ( mIncidenceMore != 0 ) // IncidenceMore was set *before* general.
    initIncidenceMore();
}

void IncidenceView::initIncidenceMore()
{
  Q_ASSERT( mItem.hasPayload<Incidence::Ptr>() );
  const Incidence::Ptr incidencePtr = CalendarSupport::incidence( mItem );

  IncidenceEditorsNG::IncidenceEditor *editor = new IncidenceEditorsNG::IncidenceCategories( mIncidenceMore->mUi );
  mEditor->combine( editor );

  editor = new IncidenceEditorsNG::IncidenceDescription( mIncidenceMore->mUi );
  mEditor->combine( editor );

  editor = new IncidenceEditorsNG::IncidenceAttendee( 0, mEditorDateTime, mIncidenceMore->mUi );
  mEditor->combine( editor );

  editor = new IncidenceEditorsNG::IncidenceAlarm( mEditorDateTime, mIncidenceMore->mUi );
  mEditor->combine( editor );

  Q_ASSERT( mEditorDateTime != 0 );
  editor = new IncidenceEditorsNG::IncidenceRecurrence( mEditorDateTime, mIncidenceMore->mUi );
  mEditor->combine( editor );

  editor = new IncidenceEditorsNG::IncidenceSecrecy( mIncidenceMore->mUi );
  mEditor->combine( editor );

  editor = new IncidenceEditorsNG::IncidenceAttachment( mIncidenceMore->mUi );
  mEditor->combine( editor );
  mEditor->load( incidencePtr );
}

void IncidenceView::setMoreEditor( MobileIncidenceMore *editorWidget )
{
  mIncidenceMore = editorWidget;
  if ( mEditorDateTime != 0 ) // IncidenceGeneral was not set yet.
    initIncidenceMore();
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

  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( mItem );
  mEditor->save( incidence );

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
  mItemManager->save();
}

void IncidenceView::slotSaveFinished( CalendarSupport::EditorItemManager::SaveAction action )
{
  deleteLater();
}

void IncidenceView::slotSaveFailed( CalendarSupport::EditorItemManager::SaveAction action, const QString &message )
{
  QPointer<QMessageBox> dlg = new QMessageBox; //krazy:exclude=qclasses
  dlg->setIcon( QMessageBox::Warning );
  dlg->setWindowTitle( i18n( "Saving the event failed." ) );
  dlg->setInformativeText( i18n( "Reason:\n\n" ) + message );
  dlg->addButton( i18n( "Ok" ), QMessageBox::AcceptRole );
  dlg->exec();
}

void IncidenceView::cancel()
{
  deleteLater();
}

#include "incidenceview.moc"
