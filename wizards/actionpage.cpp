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

#include <qapplication.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbutton.h>

#include <kaccelmanager.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "servertypemanager.h"

#include "actionpage.h"

ActionPage::ActionPage( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *layout = new QGridLayout( this, 8, 2, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mTitleLabel = new QLabel( this );

  mConnectionBox = new QListBox( this );

  mAddButton = new QPushButton( i18n( "New" ), this );
  mEditButton = new QPushButton( i18n( "Edit..." ), this );
  mEditButton->setEnabled( false );
  mDeleteButton = new QPushButton( i18n( "Delete" ), this );
  mDeleteButton->setEnabled( false );
  mActivateButton = new QPushButton( this );
  mActivateButton->setEnabled( false );

  QFrame *hline = new QFrame( this );
  hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );

  QPushButton *quitButton = new QPushButton( i18n( "Quit" ), this );

  layout->addMultiCellWidget( mTitleLabel, 0, 0, 0, 1 );
  layout->addMultiCellWidget( mConnectionBox, 1, 5, 0, 0 );
  layout->addWidget( mAddButton, 1, 1 );
  layout->addWidget( mEditButton, 2, 1 );
  layout->addWidget( mDeleteButton, 3, 1 );
  layout->addWidget( mActivateButton, 4, 1 );
  layout->addMultiCellWidget( hline, 6, 6, 0, 1 );
  layout->addWidget( quitButton, 7, 1 );

  connect( mConnectionBox, SIGNAL( selectionChanged() ),
           this, SLOT( selectionChanged() ) );
  connect( mAddButton, SIGNAL( clicked() ),
           this, SLOT( addConnection() ) );
  connect( mEditButton, SIGNAL( clicked() ),
           this, SLOT( editConnection() ) );
  connect( mDeleteButton, SIGNAL( clicked() ),
           this, SLOT( deleteConnection() ) );
  connect( mActivateButton, SIGNAL( clicked() ),
           this, SLOT( activateConnection() ) );
  connect( quitButton, SIGNAL( clicked() ),
           qApp, SLOT( quit() ) );

  KAcceleratorManager::manage( this );
}

ActionPage::~ActionPage()
{
}

void ActionPage::setServerType( const QString &serverType )
{
  mServerType = ServerTypeManager::self()->serverType( serverType );
  mTitleLabel->setText( i18n( "All available  connections for '%1'" )
                        .arg( ServerTypeManager::self()->title( serverType ) ) );

  reloadConnections();
}

void ActionPage::selectionChanged()
{
  bool state = (mConnectionBox->selectedItem() != 0);

  mEditButton->setEnabled( state );
  mDeleteButton->setEnabled( state );
  mActivateButton->setEnabled( state );

  QString activeMsg = i18n( "Set Passive..." );
  QString passiveMsg = i18n( "Set Active..." );

  if ( state ) {
    int counter = 0;

    ServerType::ConnectionInfoList::ConstIterator it;
    for ( it = mConnectionInfoList.begin(); it != mConnectionInfoList.end(); ++it ) {
      if ( counter == mConnectionBox->currentItem() ) {
        if ( (*it).active )
          mActivateButton->setText( activeMsg );
        else
          mActivateButton->setText( passiveMsg );
        break;
      }

      counter++;
    }
  } else
    mActivateButton->setText( passiveMsg );
}

void ActionPage::addConnection()
{
  mServerType->addConnection();

  reloadConnections();
}

void ActionPage::editConnection()
{
  int counter = 0;

  ServerType::ConnectionInfoList::ConstIterator it;
  for ( it = mConnectionInfoList.begin(); it != mConnectionInfoList.end(); ++it ) {
    if ( counter == mConnectionBox->currentItem() ) {
      mServerType->editConnection( (*it).uid );
      break;
    }

    counter++;
  }

  reloadConnections();
}

void ActionPage::deleteConnection()
{
  int counter = 0;

  QString msg = i18n( "Do you really want to delete this connection?" );

  ServerType::ConnectionInfoList::ConstIterator it;
  for ( it = mConnectionInfoList.begin(); it != mConnectionInfoList.end(); ++it ) {
    if ( counter == mConnectionBox->currentItem() ) {
      if ( KMessageBox::questionYesNoList( 0, msg, (*it).name ) == KMessageBox::Yes )
        mServerType->deleteConnection( (*it).uid );
      break;
    }

    counter++;
  }

  reloadConnections();
}

void ActionPage::activateConnection()
{
  int counter = 0;

  ServerType::ConnectionInfoList::ConstIterator it;
  for ( it = mConnectionInfoList.begin(); it != mConnectionInfoList.end(); ++it ) {
    if ( counter == mConnectionBox->currentItem() ) {
      mServerType->activateConnection( (*it).uid, !(*it).active );
      break;
    }

    counter++;
  }

  reloadConnections();
}

void ActionPage::reloadConnections()
{
  Q_ASSERT( mServerType );

  mConnectionBox->clear();

  QPixmap connected = KGlobal::iconLoader()->loadIcon( "connect_established", KIcon::Small );
  QPixmap disconnected = KGlobal::iconLoader()->loadIcon( "connect_no", KIcon::Small );

  mConnectionInfoList = mServerType->connectionInfo();
  ServerType::ConnectionInfoList::ConstIterator it;
  for ( it = mConnectionInfoList.begin(); it != mConnectionInfoList.end(); ++it ) {
    QString name = (*it).name + " " + ((*it).active ? i18n( "(active)" ) : "(passive)");
    mConnectionBox->insertItem( ((*it).active ? connected : disconnected) , name );
  }

  selectionChanged();
}

#include "actionpage.moc"
