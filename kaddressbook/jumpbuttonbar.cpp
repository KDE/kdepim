/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include "jumpbuttonbar.h"

#include <qevent.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qstring.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

JumpButtonBar::JumpButtonBar( QWidget *parent, const char *name )
  : QVBox( parent, name )
{
  // I don't think this is i18n approved, but I am not sure.
  QPushButton *b;
  QString letter;

  mUpButton = new QPushButton( this );
  mUpButton->setPixmap( KGlobal::iconLoader()->loadIcon( "up", KIcon::Small ) );
  connect( mUpButton, SIGNAL( clicked() ), this, SLOT( upClicked() ) );

  mScrollView = new QScrollView( this );
  mScrollView->setVScrollBarMode( QScrollView::AlwaysOff );
  mScrollView->setHScrollBarMode( QScrollView::AlwaysOff );

  QVBox *vBox = new QVBox( mScrollView->viewport() );
  mScrollView->addChild( vBox );
  
  b = new QPushButton( "0,1,2", vBox, "0" );
  connect( b, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
  
  for ( int i = 'a'; i <= 'z'; i++ ) {
    letter = (char)i;
    b = new QPushButton( letter, vBox, letter.latin1() );
    connect( b, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
  }
  
  vBox->setFixedSize( vBox->sizeHint() );

  // There has to be a better way of setting the preferred size of the 
  // scroll view. Hmmm.
  mScrollView->setFixedWidth( vBox->sizeHint().width() + 3 );
  
  mDownButton = new QPushButton( this );
  mDownButton->setPixmap( KGlobal::iconLoader()->loadIcon( "down", KIcon::Small ) );
  connect( mDownButton, SIGNAL( clicked() ), this, SLOT( downClicked() ) );
  
  // insert a spacer widget to use the rest of the space
  new QWidget( this );
}

JumpButtonBar::~JumpButtonBar()
{
}

QSizePolicy JumpButtonBar::sizePolicy() const
{
  return QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Minimum,
                      QSizePolicy::Vertically );
}
    
void JumpButtonBar::upClicked()
{
  mScrollView->scrollBy( 0, -25 );
  
  updateArrowButtons();
}

void JumpButtonBar::downClicked()
{
  mScrollView->scrollBy( 0, 25 );
  
  updateArrowButtons();
}

void JumpButtonBar::letterClicked()
{
  QString name = sender()->name();
  if ( !name.isEmpty() )
    emit jumpToLetter( QChar( name[0] ) );
}

void JumpButtonBar::updateArrowButtons()
{
  QScrollBar *bar = mScrollView->verticalScrollBar();
  mUpButton->setEnabled( bar->value() > bar->minValue() );
  mDownButton->setEnabled( bar->value() < bar->maxValue() );
}

void JumpButtonBar::resizeEvent( QResizeEvent *e )
{
  QVBox::resizeEvent( e );

  updateArrowButtons();
}

void JumpButtonBar::show()
{
  QVBox::show();

  updateArrowButtons();
}

#include "jumpbuttonbar.moc"
