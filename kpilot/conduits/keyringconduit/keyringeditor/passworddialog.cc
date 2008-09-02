/* passworddialog.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "passworddialog.h"

QString PasswordDialog::getPassword( QWidget* parent, Mode mode )
{
	PasswordDialog pd( parent, mode );
	pd.exec();
	
	if( pd.result() == QDialog::Accepted )
	{
		return pd.fUi.password1->text();
	}
	
	return QString();
}

PasswordDialog::PasswordDialog( QWidget *parent, Mode mode )
	: QDialog( parent )
{
	fMode = mode;
	
	fUi.setupUi(this);
	fUi.equalLabel->setVisible( false );
	
	if( mode == New )
	{
		connect( fUi.password1, SIGNAL( textChanged( const QString& ) )
			, this, SLOT( checkPasswords() ) );
		connect( fUi.password2, SIGNAL( textChanged( const QString& ) )
			, this, SLOT( checkPasswords() ) );
	}
	else
	{
		connect( fUi.password1, SIGNAL( textChanged( const QString& ) )
			, this, SLOT( checkPassword() ) );
		
		fUi.password2->setVisible( false );
		fUi.label_2->setVisible( false );
	}
}

void PasswordDialog::checkPasswords()
{
	if( fUi.password1->text() == fUi.password2->text() )
	{
		fUi.equalLabel->setText( "Passwords equal!" );
		fUi.equalLabel->setVisible( true );
	}
	else
	{
		fUi.equalLabel->setText( "Passwords not equal!" );
		fUi.equalLabel->setVisible( true );
	}
}

void PasswordDialog::checkPassword()
{
	if( fUi.password1->text().isEmpty() )
	{
		fUi.equalLabel->setText( "Password empty" );
		fUi.equalLabel->setVisible( true );
	}
	else
	{
		fUi.equalLabel->setVisible( false );
	}
}

void PasswordDialog::accept()
{
	if( fMode == New )
	{
		if( !fUi.password1->text().isEmpty()
			&& fUi.password1->text() == fUi.password2->text() )
		{
			done( QDialog::Accepted );
		}
		else
		{
			checkPasswords();
		}
	}
	else
	{
		if( !fUi.password1->text().isEmpty() )
		{
			done( QDialog::Accepted );
		}
		else
		{
			fUi.equalLabel->setText( "Password empty!" );
			fUi.equalLabel->setVisible( true );
		}
	}
}
