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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtooltip.h>

#include <kaccelmanager.h>
#include <kbuttonbox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpassdlg.h>

#include "addhostdialog.h"

AddHostDialog::AddHostDialog( QWidget* parent,  const char* name )
  : KDialogBase( Plain, i18n( "Add Host" ), Ok | Cancel, Ok, parent, name, true, true )
{
  QWidget *page = plainPage();

  QGridLayout *layout = new QGridLayout( page, 3, 5, 0, spacingHint() );

  mHostEdit = new KLineEdit( page );
  layout->addMultiCellWidget( mHostEdit, 0, 0, 1, 2 );
  connect( mHostEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotHostEditChanged( const QString& ) ) );

  QLabel *label = new QLabel( i18n( "Host:" ), page );
  label->setBuddy( mHostEdit );
  layout->addWidget( label, 0, 0 );

  mPortSpinBox = new QSpinBox( page );
  mPortSpinBox->setMaxValue( 65535 );
  mPortSpinBox->setValue( 389 );
  layout->addWidget( mPortSpinBox, 1, 1 );

  label = new QLabel( i18n( "Port:" ), page );
  QToolTip::add( label, i18n( "The port number of the directory server if it is using a non-standard port (389 is the standard)" ) );
  label->setBuddy( mPortSpinBox );
  layout->addWidget( label, 1, 0 );

  mBaseEdit = new KLineEdit( page );
  layout->addMultiCellWidget( mBaseEdit, 2, 2, 1, 2 );

  label = new QLabel( i18n( "Base DN:" ), page );
  QToolTip::add( label, i18n( "The base DN used for searching" ) );
  label->setBuddy( mBaseEdit );
  layout->addWidget( label, 2, 0 );

  mBindEdit = new KLineEdit( page );
  layout->addMultiCellWidget( mBindEdit, 3, 3, 1, 2 );

  label = new QLabel( i18n( "Bind DN:" ), page );
  QToolTip::add( label, i18n( "The bind DN used for searching" ) );
  label->setBuddy( mBindEdit );
  layout->addWidget( label, 3, 0 );

  mPwdBindEdit = new KPasswordEdit( page );
  layout->addMultiCellWidget( mPwdBindEdit, 4, 4, 1, 2 );

  label = new QLabel( i18n( "Password:" ), page );
  QToolTip::add( label, i18n( "The password used for searching" ) );
  label->setBuddy( mPwdBindEdit );
  layout->addWidget( label, 4, 0 );

  resize( QSize( 380, 150 ).expandedTo( sizeHint() ) );
  enableButtonOK( !mHostEdit->text().isEmpty());
  mHostEdit->setFocus();

  KAcceleratorManager::manage( this );
}

AddHostDialog::~AddHostDialog()
{
}

void AddHostDialog::slotHostEditChanged( const QString &text )
{
  enableButtonOK( !text.isEmpty() );
}

void AddHostDialog::setHost( const QString &host )
{
  mHostEdit->setText( host );
}

void AddHostDialog::setPort( int port )
{
  mPortSpinBox->setValue( port );
}

void AddHostDialog::setBaseDN( const QString &baseDN )
{
  mBaseEdit->setText( baseDN );
}

void AddHostDialog::setBindDN( const QString &bindDN )
{
  mBindEdit->setText( bindDN );
}

void AddHostDialog::setPwdBindDN( const QString &pwdBindDN )
{
  mPwdBindEdit->setText( pwdBindDN );
}

QString AddHostDialog::host() const
{
  return mHostEdit->text().stripWhiteSpace();
}

int AddHostDialog::port() const
{
  return mPortSpinBox->value();
}

QString AddHostDialog::baseDN() const
{
  return mBaseEdit->text().stripWhiteSpace();
}

QString AddHostDialog::bindDN() const
{
  return mBindEdit->text().stripWhiteSpace();
}

QString AddHostDialog::pwdBindDN() const
{
  return mPwdBindEdit->text().stripWhiteSpace();
}

#include "addhostdialog.moc"
