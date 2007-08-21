/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include <kdialog.h>
#include <klocale.h>

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

#include "addresseediffalgo.h"
#include "genericdiffalgo.h"
#include "htmldiffalgodisplay.h"
#include "memberinfo.h"

#include "singleconflictdialog.h"

SingleConflictDialog::SingleConflictDialog( QSync::SyncMapping &mapping, QWidget *parent )
  : ConflictDialog( mapping, parent ), mDiffAlgo( 0 )
{
  initGUI();

  QString format = mapping.changeAt( 0 ).objectFormatName();
  QSync::SyncChange leftChange = mapping.changeAt( 0 );
  QSync::SyncChange rightChange = mapping.changeAt( 1 );

  if ( format == "file" ) {
    mDiffAlgo = new KSync::GenericDiffAlgo( leftChange.data(), rightChange.data() );
  } else if ( format == "vcard" ) {
  } else if ( format == "calendar" ) {
  } else if ( format == "xml-contact" ) {
    mDiffAlgo = new KSync::AddresseeDiffAlgo( leftChange.data(), rightChange.data() );
  }

// TODO: SyncChange doesn't have member as struct member anymore...
// Use SyncMapping to determine the member ... see msynctool example
#if 0
  MemberInfo miLeft( leftChange.member() );
  mDiffAlgoDisplay->setLeftSourceTitle( miLeft.name() );
  MemberInfo miRight( rightChange.member() );
  mDiffAlgoDisplay->setRightSourceTitle( miRight.name() );
#endif
  if ( mDiffAlgo ) {
    mDiffAlgo->addDisplay( mDiffAlgoDisplay );
    mDiffAlgo->run();
  }
}

SingleConflictDialog::~SingleConflictDialog()
{
  delete mDiffAlgo;
  mDiffAlgo = 0;
}

void SingleConflictDialog::useFirstChange()
{
  mMapping.solve( mMapping.changeAt( 0 ) );

  accept();
}

void SingleConflictDialog::useSecondChange()
{
  mMapping.solve( mMapping.changeAt( 1 ) );

  accept();
}

void SingleConflictDialog::duplicateChange()
{
  mMapping.duplicate();

  accept();
}

void SingleConflictDialog::ignoreChange()
{
  mMapping.ignore();

  accept();
}

void SingleConflictDialog::initGUI()
{
  QGridLayout *layout = new QGridLayout( this );
  layout->setMargin( KDialog::marginHint() );
  layout->setSpacing( KDialog::spacingHint() );

  layout->addWidget( new QLabel( i18n( "A conflict has appeared, please solve it manually." ), this ), 0, 0, 1, 4 );
  mDiffAlgoDisplay = new KSync::HTMLDiffAlgoDisplay( this );

  layout->addWidget( mDiffAlgoDisplay, 1, 1, 1, 2 );

  QPushButton *button = new QPushButton( i18n( "Use Item" ), this );
  connect( button, SIGNAL( clicked() ), SLOT( useFirstChange() ) );
  layout->addWidget( button, 2, 0 );

  button = new QPushButton( i18n( "Duplicate Items" ), this );
  connect( button, SIGNAL( clicked() ), SLOT( duplicateChange() ) );
  layout->addWidget( button, 2, 1 );

  button = new QPushButton( i18n( "Ignore Conflict" ), this );
  connect( button, SIGNAL( clicked() ), SLOT( ignoreChange() ) );
  layout->addWidget( button, 2, 2 );

  button = new QPushButton( i18n( "Use Item" ), this );
  connect( button, SIGNAL( clicked() ), SLOT( useSecondChange() ) );
  layout->addWidget( button, 2, 3 );
}

#include "singleconflictdialog.moc"
