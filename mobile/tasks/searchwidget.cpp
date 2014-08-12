/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Tobias Koenig <tobias.koenig@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "searchwidget.h"

#include "stylesheetloader.h"

#include <KCalCore/Todo>

#include <QtCore/QDate>
#include <QtCore/QDebug>
#include <qplatformdefs.h>

SearchWidget::SearchWidget( QWidget *parent )
  : QWidget( parent )
{
  mUi.setupUi( this );

  // set defaults
  mUi.inSummaries->setChecked( true );
  mUi.inDescriptions->setChecked( true );
  mUi.includeTodosWithoutDueDate->setChecked( true );
  mUi.startDate->setDate( QDate::currentDate() );
  mUi.endDate->setDate( QDate::currentDate().addYears( 1 ) );
  mUi.collectionCombo->setMimeTypeFilter( QStringList() << KCalCore::Todo::todoMimeType() );
}

QString SearchWidget::query() const
{
  //TODO create search query from dialog
  return QString();
}

DeclarativeSearchWidget::DeclarativeSearchWidget( QGraphicsItem *parent )
  : QGraphicsProxyWidget( parent ), mSearchWidget( new SearchWidget )
{
  QPalette palette = mSearchWidget->palette();
  palette.setColor( QPalette::Window, QColor( 0, 0, 0, 0 ) );
  mSearchWidget->setPalette( palette );
  StyleSheetLoader::applyStyle( mSearchWidget );

  setWidget( mSearchWidget );
  setFocusPolicy( Qt::StrongFocus );
}

DeclarativeSearchWidget::~DeclarativeSearchWidget()
{
}

QString DeclarativeSearchWidget::query() const
{
  return mSearchWidget->query();
}

