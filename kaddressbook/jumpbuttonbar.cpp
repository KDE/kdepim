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
#include <qbuttongroup.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qstyle.h>

#include <kabc/addressbook.h>
#include <kabc/field.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>

#include "core.h"

#include "jumpbuttonbar.h"

class JumpButton : public QPushButton
{
  public:
    JumpButton( const QString &firstChar, const QString &lastChar,
                const QString &charRange, QWidget *parent );
    JumpButton( const QString &text, QWidget *parent );

    QString charRange() const { return mCharRange; }

  private:
    QString mCharRange;
};

JumpButton::JumpButton( const QString &firstChar, const QString &lastChar,
                        const QString &charRange, QWidget *parent )
  : QPushButton( "", parent ), mCharRange( charRange )
{
  if ( !lastChar.isEmpty() )
    setText( QString( "%1 - %2" ).arg( firstChar.upper() ).arg( lastChar.upper() ) );
  else
    setText( firstChar.upper() );

  setToggleType( QButton::Toggle );
}

JumpButton::JumpButton( const QString &text, QWidget *parent )
  : QPushButton( text, parent ), mCharRange( "" )
{
  setToggleType( QButton::Toggle );
}

JumpButtonBar::JumpButtonBar( KAB::Core *core, QWidget *parent, const char *name )
  : QWidget( parent, name ), mCore( core )
{
  setMinimumSize( 1, 1 );

  QVBoxLayout *layout = new QVBoxLayout( this, 0, 0 );
  layout->setAlignment( Qt::AlignTop );
  layout->setAutoAdd( true );
  layout->setResizeMode( QLayout::FreeResize );

  mGroupBox = new QButtonGroup( 1, Qt::Horizontal, this );
  mGroupBox->layout()->setSpacing( 0 );
  mGroupBox->layout()->setMargin( 0 );
  mGroupBox->setFrameStyle( QFrame::NoFrame );
  mGroupBox->setExclusive( true );
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
  int currentButton = mGroupBox->selectedId();

  // the easiest way to remove all buttons ;)
  mButtons.setAutoDelete( true );
  mButtons.clear();
  mButtons.setAutoDelete( false );

  QStringList characters;

  // calculate how many buttons are possible
  QFontMetrics fm = fontMetrics();
  QPushButton *btn = new QPushButton( "", this );
  btn->hide();
  QSize buttonSize = style().sizeFromContents( QStyle::CT_PushButton, btn,
                     fm.size( ShowPrefix, "X - X") ).
                     expandedTo( QApplication::globalStrut() );
  delete btn;

  mAllButton = new JumpButton( i18n( "All" ), mGroupBox );
  connect( mAllButton, SIGNAL( clicked() ), this, SLOT( reset() ) );
  mButtons.append( mAllButton );
  mAllButton->show();

  int buttonHeight = buttonSize.height() + 12;
  uint possibleButtons = (height() / buttonHeight) - 1;

  QString character;
  KABC::AddressBook *ab = mCore->addressBook();
  KABC::AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    KABC::Field *field = 0;
    field = mCore->currentSortField();
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

  if ( characters.count() <= possibleButtons ) {
    // at first the easy case: all buttons fits in window
    for ( uint i = 0; i < characters.count(); ++i ) {
      JumpButton *button = new JumpButton( characters[ i ], QString::null,
                                           characters[ i ], mGroupBox );
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
                                             QString::null,
                                             characters[ current ], mGroupBox );
        connect( button, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
        mButtons.append( button );
        button->show();
        current++;
      } else {
        int pos = ( current + offset >= (int)characters.count() ?
                    characters.count() - 1 : current + offset - 1 );
        QString range;
        for ( int j = current; j < pos + 1; ++j )
          range.append( characters[ j ] );
        JumpButton *button = new JumpButton( characters[ current ],
                                             characters[ pos ], range, mGroupBox );
        connect( button, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
        mButtons.append( button );
        button->show();
        current = ( i + 1 ) * offset;
      }
    }
  }

  if ( currentButton != -1 )
    mGroupBox->setButton( currentButton );
  else
    mGroupBox->setButton( 0 );
}

void JumpButtonBar::reset()
{
  mGroupBox->setButton( 0 );

  QStringList list;
  list.append( "" );
  emit jumpToLetter( list );
}

void JumpButtonBar::letterClicked()
{
  JumpButton *button = (JumpButton*)sender();
  QString characters = button->charRange();

  QStringList charList;
  for ( uint i = 0; i < characters.length(); ++i )
    charList.append( QString( characters[ i ] ) );

  emit jumpToLetter( charList );
}

void JumpButtonBar::resizeEvent( QResizeEvent* )
{
  updateButtons();
}

class SortContainer
{
  public:
    SortContainer() {}
    SortContainer( const QString &string )
      : mString( string )
    {
    }

    bool operator< ( const SortContainer &cnt )
    {
      return ( QString::localeAwareCompare( mString, cnt.mString ) < 0 );
    }

    QString data() const
    {
      return mString;
    }

  private:
    QString mString;
};

void JumpButtonBar::sortListLocaleAware( QStringList &list )
{
  QValueList<SortContainer> sortList;

  QStringList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    sortList.append( SortContainer( *it ) );

  qHeapSort( sortList );
  list.clear();

  QValueList<SortContainer>::Iterator sortIt;
  for ( sortIt = sortList.begin(); sortIt != sortList.end(); ++sortIt )
    list.append( (*sortIt).data() );
}

#include "jumpbuttonbar.moc"
