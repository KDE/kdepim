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

#include <qlabel.h>

#include <kcombobox.h>
#include <klocale.h>

#include "filterselectionwidget.h"

FilterSelectionWidget::FilterSelectionWidget( QWidget *parent, const char *name )
  : QHBox( parent, name )
{
  new QLabel( i18n( "Filter:" ), this );
  
  mFilterCombo = new KComboBox( this );
  connect( mFilterCombo, SIGNAL( activated( int ) ), SLOT( activated( int ) ) );
}

FilterSelectionWidget::~FilterSelectionWidget()
{
}

int FilterSelectionWidget::currentItem() const
{
  return mFilterCombo->currentItem() - 1;
}

void FilterSelectionWidget::setCurrentItem( int index )
{
  mFilterCombo->setCurrentItem( index );
}

unsigned int FilterSelectionWidget::count() const
{
  return mFilterCombo->count();
}

QString FilterSelectionWidget::text( int index ) const
{
  return mFilterCombo->text( index );
}

void FilterSelectionWidget::setFilterNames( const QStringList &names )
{
  mFilterCombo->clear();
  mFilterCombo->insertItem( i18n( "None" ) );
  mFilterCombo->insertStringList( names );

  emit filterActivated( -1 );
}

QString FilterSelectionWidget::currentFilterName() const
{
  return mFilterCombo->currentText();
}
   
void FilterSelectionWidget::setCurrentFilterName( const QString &name )
{
  mFilterCombo->setCurrentText( name );
}

void FilterSelectionWidget::activated( int index )
{
  emit filterActivated( index - 1 );
}

#include "filterselectionwidget.moc"
