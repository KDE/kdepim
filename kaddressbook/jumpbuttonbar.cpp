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
#include <qstring.h>

#include <kabc/addressbook.h>
#include <klocale.h>

class JumpButton : public QPushButton
{
  public:
    JumpButton( const QString &text, QWidget *parent,
                const QChar &character );

    void setCharacter( const QChar &character );
    QChar character() const;

  private:
    QChar mCharacter;
};

JumpButton::JumpButton( const QString &text, QWidget *parent,
                        const QChar &character )
  : QPushButton( text, parent )
{
  mCharacter = character;
}

void JumpButton::setCharacter( const QChar &character )
{
  mCharacter = character;
}

QChar JumpButton::character() const
{
  return mCharacter;
}

JumpButtonBar::JumpButtonBar( ViewManager *parent, const char *name )
  : QWidget( parent, name )
{
  JumpButton *b;
  QString letter;

  QGridLayout *topLayout = new QGridLayout( this, 10, 3 );
  
  b = new JumpButton( "0,1,2", this, QChar( '0' ) );
  connect( b, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
  topLayout->addMultiCellWidget( b, 0, 0, 0, 1 );

  QValueList<QChar> charMap;
  KABC::AddressBook::Iterator it;
  KABC::AddressBook *ab = parent->addressBook();
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    QChar curr;
    if ( !(*it).formattedName().isEmpty() )
      curr = (*it).formattedName()[ 0 ];
    else
      curr = (*it).givenName()[ 0 ];

    if ( !charMap.contains( curr ) )
      charMap.append( curr );
  }

  int maxRows = charMap.count() / 2; // we use 2 columns
  if ( charMap.count() % 2 )
    maxRows++;

  qHeapSort( charMap );

  int row = 1, col = 0;
  for ( uint i = 0; i < charMap.count(); ++i ) {
    b = new JumpButton( charMap[ i ], this, charMap[ i ] );
    connect( b, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
    topLayout->addWidget( b, row, col );

    if ( row == maxRows ) {
      col++;
      row = 1;
    } else
      row++;
  }
}

JumpButtonBar::~JumpButtonBar()
{
}


QSizePolicy JumpButtonBar::sizePolicy() const
{
  return QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Minimum,
                      QSizePolicy::Vertically );
}
  

void JumpButtonBar::letterClicked()
{
  JumpButton *button = (JumpButton*)sender();
  QChar character = button->character();
  if ( !character.isNull() )
    emit jumpToLetter( character );
}

#include "jumpbuttonbar.moc"
