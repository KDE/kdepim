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

#include <qevent.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>

#include <kabc/addressbook.h>
#include <kabc/field.h>
#include <kdebug.h>
#include <klocale.h>

#include "jumpbuttonbar.h"

class JumpButton : public QPushButton
{
  public:
    JumpButton( const QString &text, QWidget *parent,
                const QString &character );

    void setCharacter( const QString &character );
    QString character() const;

  private:
    QString mCharacter;
};

JumpButton::JumpButton( const QString &text, QWidget *parent,
                        const QString &character )
  : QPushButton( text, parent )
{
  mCharacter = character;
}

void JumpButton::setCharacter( const QString &character )
{
  mCharacter = character;
}

QString JumpButton::character() const
{
  return mCharacter;
}

JumpButtonBar::JumpButtonBar( ViewManager *parent, const char *name )
  : QWidget( parent, name ), mViewManager( parent )
{
  mButtonLayout = new QGridLayout( this, 10, 3 );
  
  JumpButton *b = new JumpButton( "0,1,2", this, "0" );
  connect( b, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );

  mButtonLayout->addMultiCellWidget( b, 0, 0, 0, 1 );

  recreateButtons();
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
  QString character = button->character();
  if ( !character.isNull() )
    emit jumpToLetter( character );
}

void JumpButtonBar::recreateButtons()
{
  // the easiest way to remove all buttons ;)
  mButtons.setAutoDelete( true );
  mButtons.clear();
  mButtons.setAutoDelete( false );
  mCharacters.clear();

  QString character;
  KABC::AddressBook *ab = mViewManager->addressBook();
  KABC::AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    KABC::Field *field = 0;
    field = mViewManager->currentSearchField();
    if ( field && !field->value( *it ).isEmpty() )
      character = field->value( *it )[ 0 ];

    if ( !character.isEmpty() && !mCharacters.contains( character ) )
      mCharacters.append( character );
  }

  int maxRows = mCharacters.count() / 2; // we use 2 columns
  if ( mCharacters.count() % 2 )
    maxRows++;

  sortListLocaleAware( mCharacters );

  bool state = isUpdatesEnabled();
  setUpdatesEnabled( false );

  int row = 1, col = 0;
  for ( uint i = 0; i < mCharacters.count(); ++i ) {
    JumpButton *button = new JumpButton( mCharacters[ i ], this, mCharacters[ i ] );
    connect( button, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
    mButtonLayout->addWidget( button, row, col );
    mButtons.append( button );
    button->show();

    if ( row == maxRows ) {
      col++;
      row = 1;
    } else
      row++;
  }

  mButtonLayout->activate();
  setUpdatesEnabled( state );
  update();
}

void JumpButtonBar::sortListLocaleAware( QStringList &list )
{
  QStringList::Iterator beginIt = list.begin();
  QStringList::Iterator endIt = list.end();

  --endIt;
  if ( beginIt == endIt ) // don't need sorting
    return;

  QStringList::Iterator walkBackIt = endIt;
  while ( beginIt != endIt ) {
    QStringList::Iterator j1 = list.begin();
    QStringList::Iterator j2 = j1;
    ++j2;
    while ( j1 != walkBackIt ) {
      if ( QString::localeAwareCompare( *j2, *j1 ) < 0 )
        qSwap( *j1, *j2 );

      ++j1;
      ++j2;
    }
    ++beginIt;
    --walkBackIt;
  }
}

#include "jumpbuttonbar.moc"
