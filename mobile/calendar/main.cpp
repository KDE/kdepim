/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Volker Krause <vkrause@kde.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include <kdeclarativeapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <incidenceeditors/korganizereditorconfig.h>

#include <calendarviews/eventviews/eventview.h>

#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/collectionselectionproxymodel.h>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/Collection>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/ItemFetchScope>

#include <KCalCore/Event>

#include "mainview.h"

using namespace Akonadi;
using namespace CalendarSupport;
using namespace IncidenceEditors;

int main( int argc, char **argv )
{
  const QByteArray& ba = QByteArray( "korganizer-mobile" );
  const KLocalizedString name = ki18n( "KOrganizer Mobile" );
  
  // NOTE: This is necessary to avoid a crash, but will result in an empty config.
  //       To make this really configurable do something like KOrganizerEditorConfig
  //       in incidinceeditors/groupwareintegration.cpp
  EditorConfig::setEditorConfig( new KOrganizerEditorConfig );

  KAboutData aboutData( ba, ba, name, ba, name );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KDeclarativeApplication::initCmdLine();
  KDeclarativeApplication app;

  ItemFetchScope scope;
  scope.fetchFullPayload( true );
  scope.fetchAttribute<EntityDisplayAttribute>();

  ChangeRecorder mChangeRecorder;
  mChangeRecorder.setCollectionMonitored( Collection::root(), true );
  mChangeRecorder.fetchCollection( true );
  mChangeRecorder.setItemFetchScope( scope );
  mChangeRecorder.setMimeTypeMonitored( KCalCore::Event::eventMimeType(), true );

  CalendarModel calendarModel( &mChangeRecorder );
  calendarModel.setCollectionFetchStrategy( EntityTreeModel::InvisibleCollectionFetch );

  CollectionSelectionProxyModel selectionProxyModel;
  selectionProxyModel.setCheckableColumn( CalendarModel::CollectionTitle );
  selectionProxyModel.setDynamicSortFilter( true );
  selectionProxyModel.setSortCaseSensitivity( Qt::CaseInsensitive );

  QItemSelectionModel selectionModel( &selectionProxyModel );
  selectionProxyModel.setSelectionModel( &selectionModel );
  selectionProxyModel.setSourceModel( &calendarModel );

  CalendarSupport::CollectionSelection colSel( &selectionModel );
  EventViews::EventView::setGlobalCollectionSelection( &colSel );
  
  MainView view;
  view.show();

  return app.exec();
}

