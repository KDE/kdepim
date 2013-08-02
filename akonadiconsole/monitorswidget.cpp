/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "monitorswidget.h"
#include "monitorsmodel.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/qheaderview.h>

#include <akonadi/control.h>

#include <KGlobal>
#include <KSharedConfig>
#include <KConfigGroup>

MonitorsWidget::MonitorsWidget( QWidget* parent ):
  QWidget( parent )
{
  mModel = new MonitorsModel( this );

  QVBoxLayout *layout = new QVBoxLayout( this );

  mTreeView = new QTreeView( this );
  mTreeView->setModel( mModel );
  mTreeView->setAlternatingRowColors( true );
  layout->addWidget( mTreeView );

  Akonadi::Control::widgetNeedsAkonadi( this );

  KConfigGroup config( KGlobal::config(), "MonitorsWidget" );

  mTreeView->header()->restoreState( config.readEntry<QByteArray>( "state", QByteArray() ) );
}

MonitorsWidget::~MonitorsWidget()
{
  KConfigGroup config( KGlobal::config(), "MonitorsWidget" );
  config.writeEntry( "state", mTreeView->header()->saveState() );
}


#include "monitorswidget.moc"
