/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <klocale.h>

#include <qlayout.h>

#include "paireditorwidget.h"

#include "paireditordialog.h"

PairEditorDialog::PairEditorDialog( QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Pair Editor" ), Ok | Cancel, Ok,
                 parent, name, true, true )
{
  initGUI();

  setInitialSize( QSize( 300, 200 ) );
}

PairEditorDialog::~PairEditorDialog()
{
}

void PairEditorDialog::setPair( KonnectorPair *pair )
{
  mPairEditorWidget->setPair( pair );
}

KonnectorPair *PairEditorDialog::pair() const
{
  return mPairEditorWidget->pair();
}

void PairEditorDialog::initGUI()
{
  QWidget *page = plainPage();

  QVBoxLayout *layout = new QVBoxLayout( page );

  mPairEditorWidget = new PairEditorWidget( page, "PairEditorWidget" );
  layout->addWidget( mPairEditorWidget );
}

#include "paireditordialog.moc"
