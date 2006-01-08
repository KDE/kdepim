/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "confirmsavedialog.h"

#include <klocale.h>

#include <QBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

using namespace KCal;

ConfirmSaveDialog::ConfirmSaveDialog( const QString &destination,
                                      QWidget *parent, const char *name )
  : KDialogBase( parent, name, true, i18n("Confirm Save"), Ok | Cancel )
{
  QFrame *topFrame = makeMainWidget();

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel(
      i18n("You have requested to save the following objects to '%1':")
      .arg( destination ), topFrame );
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
