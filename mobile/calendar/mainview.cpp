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

#include "mainview.h"

#include <akonadi/entitytreemodel.h>
#include <akonadi/kcal/incidencemimetypevisitor.h>
#include <akonadi/kcal/calendar.h>

#include <ksystemtimezone.h>

#include <qdeclarativeengine.h>
#include <qdeclarativecontext.h>

#include "incidenceview.h"


using namespace Akonadi;
using namespace KCal;

MainView::MainView( QWidget *parent ) : KDeclarativeMainView( "korganizer-mobile", 0 /* TODO */, parent )
{
  addMimeType( IncidenceMimeTypeVisitor::eventMimeType() );

  m_calendar = new Akonadi::Calendar( entityTreeModel(), regularSelectedItems(), KSystemTimeZones::local(), this );
  engine()->rootContext()->setContextProperty( "calendarModel", QVariant::fromValue( static_cast<QObject*>( m_calendar ) ) );
}

void MainView::showRegularCalendar()
{
  m_calendar->setUnfilteredModel(regularSelectedItems());
}

void MainView::setCurrentEventItemId(qint64 id)
{
  QModelIndexList list = itemModel()->match(itemModel()->index(0, 0), EntityTreeModel::ItemIdRole, id, 1 );
  if (list.isEmpty())
    return;

  setListSelectedRow(list.first().row());
}

void MainView::newIncidence()
{
  IncidenceView *editor = new IncidenceView;
  editor->show();
}

void MainView::editIncidence( const Akonadi::Item &item, const QDate &date )
{
  IncidenceView *editor = new IncidenceView;
  editor->load( item, date );
  editor->show();
}


#include "mainview.moc"
