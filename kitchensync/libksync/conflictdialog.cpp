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

#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <libkdepim/diffalgo.h>
#include <libkdepim/htmldiffalgodisplay.h>

#include "conflictdialog.h"
#include "syncee.h"

using namespace KSync;

ConflictDialog::ConflictDialog( SyncEntry *syncEntry, SyncEntry *targetEntry,
                                QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Resolve Conflict" ), User1 | User2 | Cancel, Cancel,
                 parent, name, true, true ), mDiffAlgo( 0 )
{
  initGUI();

  mDiffAlgo = syncEntry->diffAlgo( syncEntry, targetEntry );

  mDisplay->setLeftSourceTitle( syncEntry->syncee()->title() );
  mDisplay->setRightSourceTitle( targetEntry->syncee()->title() );

  setButtonText( User1, targetEntry->syncee()->title() );
  setButtonText( User2, syncEntry->syncee()->title() );
  setButtonText( Cancel, i18n( "Keep Both" ) );

  if ( mDiffAlgo ) {
    mDiffAlgo->addDisplay( mDisplay );
    mDiffAlgo->run();
  } else {
    mDisplay->begin();
    mDisplay->conflictField( i18n( "Both entries have changed fields" ), i18n( "Unknown" ), i18n( "Unknown" ) );
    mDisplay->end();
  }

  resize( 550, 400 );
}

ConflictDialog::~ConflictDialog()
{
  delete mDiffAlgo;
  mDiffAlgo = 0;
}

void ConflictDialog::slotUser1()
{
  QDialog::done( User1 );
}

void ConflictDialog::slotUser2()
{
  QDialog::done( User2 );
}

void ConflictDialog::initGUI()
{
  QWidget *page = plainPage();

  QGridLayout *layout = new QGridLayout( page, 2, 1, marginHint(), spacingHint() );

  QLabel *label = new QLabel( i18n( "Which entry do you want to take precedence?" ), page );
  layout->addWidget( label, 0, 0, Qt::AlignCenter );

  mDisplay = new KPIM::HTMLDiffAlgoDisplay( page );
  layout->addWidget( mDisplay, 1, 0 );
}

#include "conflictdialog.moc"
