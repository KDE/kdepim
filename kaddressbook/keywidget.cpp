/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kapplication.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>

#include "keywidget.h"

KeyWidget::KeyWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *layout = new QGridLayout( this, 4, 2, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Keys:" ), this );
  layout->addWidget( label, 0, 0 );

  mKeyCombo = new KComboBox( this );
  layout->addWidget( mKeyCombo, 0, 1 );

  mAddButton = new QPushButton( i18n( "Add..." ), this );
  layout->addMultiCellWidget( mAddButton, 1, 1, 0, 1 );

  mRemoveButton = new QPushButton( i18n( "Remove" ), this );
  mRemoveButton->setEnabled( false );
  layout->addMultiCellWidget( mRemoveButton, 2, 2, 0, 1 );

  mExportButton = new QPushButton( i18n( "Export..." ), this );
  mExportButton->setEnabled( false );
  layout->addMultiCellWidget( mExportButton, 3, 3, 0, 1 );

  connect( mAddButton, SIGNAL( clicked() ), SLOT( addKey() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeKey() ) );
  connect( mExportButton, SIGNAL( clicked() ), SLOT( exportKey() ) );
}

KeyWidget::~KeyWidget()
{
}

void KeyWidget::setKeys( const KABC::Key::List &list )
{
  mKeyList = list;

  updateKeyCombo();
}

KABC::Key::List KeyWidget::keys() const
{
  return mKeyList;
}

void KeyWidget::addKey()
{
  QMap<QString, int> keyMap;
  QStringList keyTypeNames;
  QStringList existingKeyTypes;

  KABC::Key::List::ConstIterator listIt;
  for ( listIt = mKeyList.begin(); listIt != mKeyList.end(); ++listIt ) {
    if ( (*listIt).type() != KABC::Key::Custom )
      existingKeyTypes.append( KABC::Key::typeLabel( (*listIt).type() ) );
  }

  KABC::Key::TypeList typeList = KABC::Key::typeList();
  KABC::Key::TypeList::ConstIterator it;
  for ( it = typeList.begin(); it != typeList.end(); ++it ) {
    if ( (*it) != KABC::Key::Custom &&
         !existingKeyTypes.contains( KABC::Key::typeLabel( *it ) ) ) {
      keyMap.insert( KABC::Key::typeLabel( *it ), *it );
      keyTypeNames.append( KABC::Key::typeLabel( *it ) );
    }
  }

  bool ok;
  QString name = KInputDialog::getItem( i18n( "Key Type" ), i18n( "Select the key type:" ), keyTypeNames, 0, true, &ok );
  if ( !ok || name.isEmpty() )
    return;

  int type = keyMap[ name ];
  if ( !keyTypeNames.contains( name ) )
    type = KABC::Key::Custom;

  KURL url = KFileDialog::getOpenURL();
  if ( url.isEmpty() )
    return;

  QString tmpFile;
  if ( KIO::NetAccess::download( url, tmpFile, this ) ) {
    QFile file( tmpFile );
    if ( !file.open( IO_ReadOnly ) ) {
      QString text( i18n( "<qt>Unable to open file <b>%1</b>.</qt>" ) );
      KMessageBox::error( this, text.arg( url.url() ) );
      return;
    }

    QTextStream s( &file );
    QString data;

    s.setEncoding( QTextStream::UnicodeUTF8 );
    s >> data;
    file.close();

    KABC::Key key( data, type );
    if ( type == KABC::Key::Custom )
      key.setCustomTypeString( name );
    mKeyList.append( key );

    emit changed();

    KIO::NetAccess::removeTempFile( tmpFile );
  }

  updateKeyCombo();
}

void KeyWidget::removeKey()
{
  int pos = mKeyCombo->currentItem();
  if ( pos == -1 )
    return;

  QString type = mKeyCombo->currentText();
  QString text = i18n( "<qt>Do you really want to remove the key <b>%1</b>?</qt>" );
  if ( KMessageBox::warningContinueCancel( this, text.arg( type ), "", KGuiItem( i18n("&Delete"), "editdelete") ) == KMessageBox::Cancel )
    return;

  mKeyList.remove( mKeyList.at( pos ) );
  emit changed();

  updateKeyCombo();
}

void KeyWidget::exportKey()
{
  KABC::Key key = (*mKeyList.at( mKeyCombo->currentItem() ) );

  KURL url = KFileDialog::getSaveURL();

  KTempFile tempFile;
  QTextStream *s = tempFile.textStream();
  s->setEncoding( QTextStream::UnicodeUTF8 );
  (*s) << key.textData();
  tempFile.close();

  KIO::NetAccess::upload( tempFile.name(), url, kapp->mainWidget() );
}

void KeyWidget::updateKeyCombo()
{
  int pos = mKeyCombo->currentItem();
  mKeyCombo->clear();

  KABC::Key::List::ConstIterator it;
  for ( it = mKeyList.begin(); it != mKeyList.end(); ++it ) {
    if ( (*it).type() == KABC::Key::Custom )
      mKeyCombo->insertItem( (*it).customTypeString() );
    else
      mKeyCombo->insertItem( KABC::Key::typeLabel( (*it).type() ) );
  }

  mKeyCombo->setCurrentItem( pos );

  bool state = ( mKeyList.count() != 0 );
  mRemoveButton->setEnabled( state );
  mExportButton->setEnabled( state );
}

#include "keywidget.moc"
