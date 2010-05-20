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

#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include "declarativeeditors.h"

using namespace IncidenceEditorsNG;
using namespace Akonadi;

IncidenceView::IncidenceView( QWidget* parent )
  : KDeclarativeFullScreenView( QLatin1String( "incidence-editor" ), parent )
  , mCollectionCombo( 0 )
  , mEvent( new KCal::Event )
  , mEditor( new CombinedIncidenceEditor( parent ) )
{
  qmlRegisterType<DCollectionCombo>( "org.kde.incidenceeditors", 4, 5, "CollectionCombo" );
  qmlRegisterType<DIEGeneral>( "org.kde.incidenceeditors", 4, 5, "GeneralEditor" );
  qmlRegisterType<DIEDateTime>( "org.kde.incidenceeditors", 4, 5, "DateTimeEditor" );
}

IncidenceView::~IncidenceView()
{
  delete mEditor;
}

void IncidenceView::setCollectionCombo( Akonadi::CollectionComboBox *combo )
{
  mCollectionCombo = combo;
  mCollectionCombo->setMimeTypeFilter( QStringList() << Akonadi::IncidenceMimeTypeVisitor::eventMimeType() );
}

void IncidenceView::setDateTimeEditor( IncidenceDateTimeEditor *editor )
{
  mEditor->combine( editor );
  editor->load( mEvent );
}

void IncidenceView::setGeneralEditor( IncidenceGeneralEditor *editor )
{
  mEditor->combine( editor );
  editor->load( mEvent );
}

void IncidenceView::save()
{
  deleteLater();
}

void IncidenceView::cancel()
{
  deleteLater();
}
