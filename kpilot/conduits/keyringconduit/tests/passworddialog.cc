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
