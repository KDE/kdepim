/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "confirmsavedialog.h"

#include <klocale.h>

#include <QBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

using namespace KCal;

ConfirmSaveDialog::ConfirmSaveDialog( const QString &destination,
                                      QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18n( "Confirm Save" ) );
  setModal( true );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  QFrame *topFrame = new QFrame( this );
  setMainWidget( topFrame );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel(
      i18n("You have requested to save the following objects to '%1':",
        destination ), topFrame );
  topLayout->addWidget( label );

  QStringList headers;
  headers << i18n("Operation") << i18n("Type") << i18n("Summary") << i18n("UID");

  mListView = new QTreeWidget( topFrame );
  mListView->setColumnCount( 4 );
  mListView->setHeaderLabels( headers );

  topLayout->addWidget( mListView );
}

void ConfirmSaveDialog::addIncidences( const Incidence::List &incidences,
                                       const QString &operation )
{
  Incidence::List::ConstIterator it;
  for( it = incidences.begin(); it != incidences.end(); ++it ) {
    Incidence *i = *it;
    QTreeWidgetItem *item = new QTreeWidgetItem( mListView );
    item->setText( 0, operation );
    item->setText( 1, i->type() );
    item->setText( 2, i->summary() );
    item->setText( 3, i->uid() );
  }
}
