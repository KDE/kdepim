/*
    This file is part of kdepim.

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "confirmsavedialog.h"

#include <klistview.h>
#include <klocale.h>

#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>

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
  
  mListView = new KListView( topFrame );
  mListView->addColumn( i18n("Operation") );
  mListView->addColumn( i18n("Type") );
  mListView->addColumn( i18n("Summary") );
  mListView->addColumn( i18n("UID") );
  topLayout->addWidget( mListView );
}

void ConfirmSaveDialog::addIncidences( const Incidence::List &incidences,
                                       const QString &operation )
{
  Incidence::List::ConstIterator it;
  for( it = incidences.begin(); it != incidences.end(); ++it ) {
    Incidence *i = *it;
    KListViewItem *item = new KListViewItem( mListView );
    item->setText( 0, operation );
    item->setText( 1, i->type() );
    item->setText( 2, i->summary() );
    item->setText( 3, i->uid() );
  }
}
