/*
    This file is part of kdepim.

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

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include <kaccelmanager.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

#include "servertypemanager.h"

#include "overviewpage.h"

OverViewPage::OverViewPage( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *layout = new QGridLayout( this, 7, 4, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  QString msg = i18n( "KDE Groupware Wizard" );
  QLabel *label = new QLabel( "<qt><b><u><h2>" + msg + "</h2></u></b></qt>" , this );
  layout->addMultiCellWidget( label, 0, 0, 0, 2 );

  label = new QLabel( this );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "network", KIcon::Desktop ) );
  layout->addWidget( label, 0, 3 );

  label = new QLabel( "", this );
  layout->addWidget( label, 1, 0 );
  layout->setRowSpacing( 1, 20 );

  label = new QLabel( i18n( "Select the type of server you want connect your KDE to" ), this );
  layout->addMultiCellWidget( label, 2, 2, 0, 3 );

  mServerTypeGroup = new QButtonGroup( 1, Vertical, this );
  mServerTypeGroup->setFrameStyle( QFrame::NoFrame );
  layout->addMultiCellWidget( mServerTypeGroup, 3, 3, 1, 3 );

  QFrame *frame = new QFrame( this );
  frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  layout->addMultiCellWidget( frame, 4, 4, 0, 3 );

  QPushButton *nextButton = new QPushButton( i18n( "Next >" ), this );
  layout->addWidget( nextButton, 5, 2 );

  QPushButton *cancelButton = new QPushButton( i18n( "Cancel" ), this );
  layout->addWidget( cancelButton, 5, 3 );

  connect( nextButton, SIGNAL( clicked() ), this, SLOT( nextClicked() ) );
  connect( cancelButton, SIGNAL( clicked() ), this, SIGNAL( cancel() ) );

  layout->setRowStretch( 3, 1 );

  loadTypes();

  KAcceleratorManager::manage( this );
}

OverViewPage::~OverViewPage()
{
}

void OverViewPage::nextClicked()
{
  if ( mIdentifiers.count() == 0 )
    return;

  QButton *button = mServerTypeGroup->selected();
  int pos = (button ? mServerTypeGroup->id( button ) : 0 );

  emit serverTypeSelected( mIdentifiers[ pos ] );
}

void OverViewPage::loadTypes()
{
  mIdentifiers = ServerTypeManager::self()->identifiers();
  QStringList::Iterator it;

  uint counter = 0;
  for ( it = mIdentifiers.begin(); it != mIdentifiers.end(); ++it, ++counter )
    mServerTypeGroup->insert( new QRadioButton( ServerTypeManager::self()->title( *it ), mServerTypeGroup ), counter );

  if ( mServerTypeGroup->count() > 0 )
    mServerTypeGroup->find( 0 )->setDown( true );
}

#include "overviewpage.moc"

