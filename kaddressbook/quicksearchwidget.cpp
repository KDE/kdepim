/*
    This file is part of KContactManager.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "quicksearchwidget.h"

#include <QtGui/QVBoxLayout>

#include <klineedit.h>
#include <klocale.h>

QuickSearchWidget::QuickSearchWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( 0 );

  mEdit = new KLineEdit;
  mEdit->setClickMessage( i18n( "Search" ) );
  mEdit->setClearButtonShown( true );

  layout->addWidget( mEdit );

  connect( mEdit, SIGNAL( textChanged( const QString& ) ), SIGNAL( filterStringChanged( const QString& ) ) );
}


QuickSearchWidget::~QuickSearchWidget()
{
}

QSize QuickSearchWidget::sizeHint() const
{
  const QSize size = mEdit->sizeHint();
  return QSize( 200, size.height() );
}

#include "quicksearchwidget.moc"
