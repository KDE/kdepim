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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qapplication.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qstyle.h>

#include <kabc/addressbook.h>
#include <kabc/field.h>
#include <kdebug.h>
#include <klocale.h>

#include "kabcore.h"

#include "jumpbuttonbar.h"

class JumpButton : public QPushButton
{
  public:
    JumpButton( const QString &firstChar, const QString &lastChar,
                QWidget *parent );

    QString firstCharacter() const;
    QString lastCharacter() const;

  private:
    QString mFirstCharacter;
    QString mLastCharacter;
};

JumpButton::JumpButton( const QString &firstChar, const QString &lastChar,
                        QWidget *parent )
  : QPushButton( "", parent ), mFirstCharacter( firstChar ),
    mLastCharacter( lastChar )
{
  if ( !lastChar.isEmpty() )
    setText( QString( "%1 - %2" ).arg( firstChar.upper() ).arg( lastChar.upper() ) );
  else
    setText( firstChar.upper() );
}

QString JumpButton::firstCharacter() const
{
  return mFirstCharacter;
}

QString JumpButton::lastCharacter() const
{
  return mLastCharacter;
}

JumpButtonBar::JumpButtonBar( KABCore *core, QWidget *parent, const char *name )
  : QWidget( parent, name ), mCore( core )
{
  setMinimumSize( 1, 1 );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setAlignment( Qt::AlignTop );
  layout->setAutoAdd( true );
  layout->setResizeMode( QLayout::FreeResize );
}

JumpButtonBar::~JumpButtonBar()
{
}

QSizePolicy JumpButtonBar::sizePolicy() const
{
	return QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Minimum,
                      QSizePolicy::Vertically );
}

void JumpButtonBar::updateButtons()
{
  // the easiest way to remove all buttons ;)
  mButtons.setAutoDelete( true );
  mButtons.clear();
  mButtons.setAutoDelete( false );

  QStringList characters;

  // calculate how many buttons are possible
  QFontMetrics fm = fontMetrics();
  QSize buttonSize = style().sizeFromContents( QStyle::CT_PushButton, this,
                     fm.size( ShowPrefix, "X") ).
                     expandedTo( QApplication::globalStrut() );

  uint possibleButtons = height() / buttonSize.height();

  QString character;
  KABC::AddressBook *ab = mCore->addressBook();
  KABC::AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    KABC::Field *field = 0;
    field = mCore->currentSearchField();
    if ( field ) {
      setEnabled( true );
      if ( !field->value( *it ).isEmpty() )
        character = field->value( *it )[ 0 ].lower();
    } else {
      setEnabled( false );
      return;
    }

    if ( !character.isEmpty() && !characters.contains( character ) )
      characters.append( character );
  }

  sortListLocaleAware( characters );

  bool state = isUpdatesEnabled();
  setUpdatesEnabled( false );

  if ( characters.count() <= possibleButtons ) {
    // at first the easy case: all buttons fits in window
    for ( uint i = 0; i < characters.count(); ++i ) {
      JumpButton *button = new JumpButton( characters[ i ], QString::null, this );
      connect( button, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
      mButtons.append( button );
      button->show();
    }
  } else {
    if ( possibleButtons == 0 ) // to avoid crashes on startup
      return;
    int offset = characters.count() / possibleButtons;
    int odd = characters.count() % possibleButtons;
    if ( odd )
      offset++;

    int current = 0;
    for ( uint i = 0; i < possibleButtons; ++i ) {
      if ( characters.count() - current == 0 )
        continue;
      if ( characters.count() - current <= possibleButtons - i ) {
        JumpButton *button = new JumpButton( characters[ current ],
			                     QString::null, this );
        connect( button, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
        mButtons.append( button );
        button->show();
        current++;      
      } else {
        int pos = ( current + offset >= characters.count() ?
                    characters.count() - 1 : current + offset - 1 );
        JumpButton *button = new JumpButton( characters[ current ],
			                     characters[ pos ], this );
        connect( button, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
        mButtons.append( button );
        button->show();
        current = ( i + 1 ) * offset;
      }
    }
  }

  setUpdatesEnabled( state );
  update();
}

void JumpButtonBar::letterClicked()
{
  JumpButton *button = (JumpButton*)sender();
  QString character = button->firstCharacter();
  if ( !character.isEmpty() )
    emit jumpToLetter( character );
}

void JumpButtonBar::resizeEvent( QResizeEvent* )
{
  updateButtons();
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
