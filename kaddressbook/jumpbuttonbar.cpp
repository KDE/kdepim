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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QList>
#include <QPushButton>
#include <QResizeEvent>
#include <QString>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>
#include <QVBoxLayout>

#include <kabc/addressbook.h>
#include <kabc/field.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>

#include "core.h"

#include "jumpbuttonbar.h"

static bool localAwareLessThan( const QString &left, const QString &right )
{
  return ( QString::localeAwareCompare( left, right ) < 0 );
}

static QSize calculateButtonSize( QWidget *parent )
{
  QStyleOption opt;
  QFontMetrics fm = parent->fontMetrics();

  QPushButton *button = new QPushButton( "", parent );
  button->hide();

  const QSize buttonSize = parent->style()->sizeFromContents( QStyle::CT_PushButton, &opt,
                           fm.size( Qt::TextShowMnemonic, " X - X " ).expandedTo( QApplication::globalStrut() ),
                           button );
  delete button;

  return buttonSize;
}

class JumpButton : public QPushButton
{
  public:
    JumpButton( const QString &firstChar, const QString &lastChar,
                QWidget *parent );

    QString firstChar() const { return mChar; }

  private:
    QString mChar;
};

JumpButton::JumpButton( const QString &firstChar, const QString &lastChar,
                        QWidget *parent )
  : QPushButton( "", parent ), mChar( firstChar )
{
  setCheckable( true );

  if ( !lastChar.isEmpty() )
    setText( QString( "%1 - %2" ).arg( firstChar.toUpper() ).arg( lastChar.toUpper() ) );
  else
    setText( firstChar.toUpper() );

  static QSize buttonSize;
  if ( !buttonSize.isValid() )
    buttonSize = calculateButtonSize( this );

  setFixedWidth( buttonSize.width() + 10 );  // +10 for buggy oxygen style
  setMinimumHeight( 1 );
}


JumpButtonBar::JumpButtonBar( KAB::Core *core, QWidget *parent, const char *name )
  : QWidget( parent ), mCore( core ), mButtonsUpdated( true )
{
  setObjectName( name );

  mLayout = new QVBoxLayout( this );
  mLayout->setMargin( 5 );
  mLayout->setSpacing( 5 );

  mButtonGroup = new QButtonGroup;
  mButtonGroup->setExclusive( true );
}

JumpButtonBar::~JumpButtonBar()
{
}

void JumpButtonBar::updateButtons()
{
  mButtonsUpdated = true;

  int currentButton = mButtonGroup->checkedId();

  // the easiest way to remove all buttons ;)
  qDeleteAll(mButtons);
  mButtons.clear();

  QStringList characters;

  // calculate how many buttons are possible

  int buttonHeight = calculateButtonSize( this ).height() + 8;
  int possibleButtons = (height() / buttonHeight) - 1;

  // collect all characters that could be appear
  QString character;
  KABC::AddressBook *ab = mCore->addressBook();
  KABC::AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    KABC::Field *field = 0;
    field = mCore->currentSortField();
    if ( field ) {
      setEnabled( true );
      if ( !field->value( *it ).isEmpty() )
        character = field->value( *it )[ 0 ].toLower();
    } else {
      setEnabled( false );
      return;
    }

    if ( !character.isEmpty() && !characters.contains( character ) )
      characters.append( character );
  }

  // sort the collected characters locale aware
  qSort( characters.begin(), characters.end(), localAwareLessThan );

  // create a new button for every character
  if ( characters.count() <= possibleButtons ) {
    // at first the easy case: all buttons fits in window, so we assign one character per button
    for ( int i = 0; i < characters.count(); ++i ) {
      JumpButton *button = new JumpButton( characters[ i ], QString(),
                                           this );
      mLayout->addWidget( button );
      mButtonGroup->addButton( button, mButtonGroup->buttons().size() );
      connect( button, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
      mButtons.append( button );
    }
  } else {
    // there is not enough space to put every character on its own button,
    // so we create sequences (e.g. 'A - C', 'D - F')

    if ( possibleButtons == 0 ) // to avoid crashes on startup
      return;

    int offset = characters.count() / possibleButtons;
    int odd = characters.count() % possibleButtons;
    if ( odd )
      offset++;

    int current = 0;
    for ( int i = 0; i < possibleButtons; ++i ) {
      if ( characters.count() - current == 0 )
        continue;
      if ( characters.count() - current <= possibleButtons - i ) {
        JumpButton *button = new JumpButton( characters[ current ],
                                             QString(), this );
        mLayout->addWidget( button );
        mButtonGroup->addButton( button, mButtonGroup->buttons().size() );
        connect( button, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
        mButtons.append( button );
        current++;
      } else {
        int pos = ( current + offset >= (int)characters.count() ?
                    characters.count() - 1 : current + offset - 1 );
        QString range;
        for ( int j = current; j < pos + 1; ++j )
          range.append( characters[ j ] );

        JumpButton *button = new JumpButton( characters[ current ],
                                             characters[ pos ], this );
        mLayout->addWidget( button );
        mButtonGroup->addButton( button, mButtonGroup->buttons().size() );
        connect( button, SIGNAL( clicked() ), this, SLOT( letterClicked() ) );
        mButtons.append( button );
        current = ( i + 1 ) * offset;
      }
    }
  }

  if ( currentButton != -1 )
    mButtonGroup->button( currentButton )->setChecked( true );
  else if ( mButtonGroup->buttons().size() )
    mButtonGroup->button( 0 )->setChecked( true );

  mLayout->activate();
}

void JumpButtonBar::letterClicked()
{
  JumpButton *button = (JumpButton*)sender();
  QString character = button->firstChar();

  emit jumpToLetter( character );
}

void JumpButtonBar::resizeEvent( QResizeEvent* )
{
  if ( mButtonsUpdated ) {
    mButtonsUpdated = false;
    QTimer::singleShot( 0, this, SLOT( updateButtons() ) );
  }
}

#include "jumpbuttonbar.moc"
