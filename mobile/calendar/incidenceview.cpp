/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

#include <KDialog>

#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include "declarativeeditors.h"

using namespace Akonadi;
using namespace IncidenceEditorsNG;
using namespace KCal;

IncidenceView::IncidenceView( QWidget* parent )
  : KDeclarativeFullScreenView( QLatin1String( "incidence-editor" ), parent )
  , mCollectionCombo( 0 )
  , mEditor( new CombinedIncidenceEditor( parent ) )
{
  qmlRegisterType<DCollectionCombo>( "org.kde.incidenceeditors", 4, 5, "CollectionCombo" );
  qmlRegisterType<DIEGeneral>( "org.kde.incidenceeditors", 4, 5, "GeneralEditor" );
  qmlRegisterType<DIEDateTime>( "org.kde.incidenceeditors", 4, 5, "DateTimeEditor" );

  mItem.setPayload<KCal::Incidence::Ptr>( KCal::Incidence::Ptr( new KCal::Event ) );
}

IncidenceView::~IncidenceView()
{
  delete mEditor;
}

void IncidenceView::load( const Akonadi::Item &item, const QDate &date )
{
  Q_ASSERT( item.hasPayload() ); // TODO: Fetch payload if there is no payload set.
  
  mItem = item;
  mEditor->load( mItem.payload<Incidence::Ptr>() );
  mActiveDate = date;

  if ( mCollectionCombo )
    mCollectionCombo->setDefaultCollection( mItem.parentCollection() );
}

void IncidenceView::setCollectionCombo( Akonadi::CollectionComboBox *combo )
{
  mCollectionCombo = combo;
  mCollectionCombo->setMimeTypeFilter( QStringList() << Akonadi::IncidenceMimeTypeVisitor::eventMimeType() );
  mCollectionCombo->setDefaultCollection( mItem.parentCollection() );
}

void IncidenceView::setDateTimeEditor( IncidenceDateTimeEditor *editor )
{
  mEditor->combine( editor );
  editor->setActiveDate( mActiveDate );
  editor->load( mItem.payload<Incidence::ConstPtr>() );
}

void IncidenceView::setGeneralEditor( IncidenceGeneralEditor *editor )
{
  mEditor->combine( editor );
  editor->load( mItem.payload<Incidence::Ptr>() );
}

void IncidenceView::save()
{
  if ( !mEditor->isValid() )
    return;

  KCal::Event::Ptr event( new KCal::Event );
  mEditor->save( event );

  Akonadi::Item item;
  item.setMimeType( Akonadi::IncidenceMimeTypeVisitor::eventMimeType() );
  item.setPayload<KCal::Event::Ptr>( event );
  
  Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, mCollectionCombo->currentCollection() );
  connect( job, SIGNAL(result(KJob*)), SLOT(itemCreateResult(KJob*)) );
}

void IncidenceView::cancel()
{
  deleteLater();
}

/// Private slots

void IncidenceView::itemCreateResult( KJob *job )
{
  if ( job->error() )
    kDebug() << "Event creation failed!";

  deleteLater();
}
