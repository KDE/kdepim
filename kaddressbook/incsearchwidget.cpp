/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>                   
                                                                        
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

#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>

#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>

#include "incsearchwidget.h"

IncSearchWidget::IncSearchWidget( QWidget *parent, const char* )
    : QWidget( parent, "kde toolbar widget" )
{
  setCaption( i18n( "Incremental Search" ) );

  initGUI();

  connect( mEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotAnnounce() ) );
  connect( mEdit, SIGNAL( returnPressed() ),
           SLOT( slotAnnounce() ) );
  connect( mCombo, SIGNAL( activated( const QString& ) ),
           SLOT( slotAnnounce() ) );
}

IncSearchWidget::~IncSearchWidget()
{
}

void IncSearchWidget::slotAnnounce()
{
  emit incSearch( mEdit->text(), mCombo->currentItem() );
}

void IncSearchWidget::setFields( const QStringList& fields )
{
  mCombo->clear();
  mCombo->insertStringList( fields );
}

void IncSearchWidget::initGUI()
{
  setName("kde toolbar widget");

  QHBoxLayout *layout = new QHBoxLayout( this, 2, KDialog::spacingHint() ); 

  QLabel *label = new QLabel( i18n( "Incremental search:" ), this, "kde toolbar widget" );
  label->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
  layout->addWidget( label );

  mEdit = new KLineEdit( this );
  layout->addWidget( mEdit );

  mCombo = new QComboBox( false, this );
  layout->addWidget( mCombo );

  QToolTip::add( mCombo, i18n( "Select Incremental Search Field" ) );

  resize( QSize(420, 50).expandedTo( sizeHint() ) );
}

#include "incsearchwidget.moc"
