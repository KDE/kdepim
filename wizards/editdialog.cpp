/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include "editdialog.h"

EditDialog::EditDialog( QWidget *parent, const char *name )
  : KDialogBase( Plain, "", Ok | Cancel, Ok, parent, name,
                 true, true )
{
  QFrame *page = plainPage();

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n( "Name" ), page );
  topLayout->addWidget( label, 0, 0 );
  mResourceNameEdit = new KLineEdit( page );
  topLayout->addWidget( mResourceNameEdit, 0, 1 );

  label = new QLabel( i18n( "Server Name" ), page );
  topLayout->addWidget( label, 1, 0 );
  mServerEdit = new KLineEdit( page );
  topLayout->addWidget( mServerEdit, 1, 1 );

  label = new QLabel( i18n( "Domain" ), page );
  topLayout->addWidget( label, 2, 0 );
  mDomainEdit = new KLineEdit( page );
  topLayout->addWidget( mDomainEdit, 2, 1 );

  label = new QLabel( i18n( "User Name" ), page );
  topLayout->addWidget( label, 3, 0 );
  mUserEdit = new KLineEdit( page );
  topLayout->addWidget( mUserEdit, 3, 1 );

  label = new QLabel( i18n( "Password" ), page );
  topLayout->addWidget( label, 4, 0 );
  mPasswordEdit = new KLineEdit( page );
  mPasswordEdit->setEchoMode( KLineEdit::Password );
  topLayout->addWidget( mPasswordEdit, 4, 1 );

  mUseSSLCheck = new QCheckBox( i18n( "Use SSL connection" ), page );
  topLayout->addMultiCellWidget( mUseSSLCheck, 5, 5, 0, 1 );

  topLayout->setRowStretch( 6, 1 );
}

EditDialog::~EditDialog()
{
}

void EditDialog::setResourceName( const QString& name )
{
  mResourceNameEdit->setText( name );
}

QString EditDialog::resourceName() const
{
  return mResourceNameEdit->text();
}

void EditDialog::setServer( const QString& server )
{
  mServerEdit->setText( server );
}

QString EditDialog::server() const
{
  return mServerEdit->text();
}

void EditDialog::setDomain( const QString& domain )
{
  mDomainEdit->setText( domain );
}

QString EditDialog::domain() const
{
  return mDomainEdit->text();
}

void EditDialog::setUser( const QString& user )
{
  mUserEdit->setText( user );
}

QString EditDialog::user() const
{
  return mUserEdit->text();
}

void EditDialog::setPassword( const QString& password )
{
  mPasswordEdit->setText( password );
}

QString EditDialog::password() const
{
  return mPasswordEdit->text();
}

void EditDialog::setUseSSLConnection( bool use )
{
  mUseSSLCheck->setChecked( use );
}

bool EditDialog::useSSLConnection() const
{
  return mUseSSLCheck->isChecked();
}
