/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "kornshell.h"

#include "boxcontainer.h"
#include "dockedcontainer.h"
#include "korncfgimpl.h"
#include "hvcontainer.h"
#include "password.h"
#include "settings.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <klocale.h>

KornShell::KornShell( QWidget * parent )
	: QWidget( parent ),
	_settings( Settings::self() ),
	_const_settings( _settings ),
	_box( 0 ),
	_configDialog( 0 )
{
	//_config->checkUpdate( "korn_kde_3_4_config_change", "korn-3-4-config_change.upd" );
	
	readConfig();
}

KornShell::~KornShell()
{
	delete _box;
}

void KornShell::show()
{
	_box->showBox();
}

void KornShell::optionDlg()
{
	if( _configDialog )
	{
		_configDialog->show();
		return;
	}
	
	_configDialog = new KDialog( (QWidget*)0 );
	_configDialog->setCaption( i18n( "Korn Configuration" ) );
	_configDialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
	_configDialog->setModal( false );
	_configDialog->showButtonSeparator( true );
	
	KornCfgImpl *widget = new KornCfgImpl( _configDialog );
	_configDialog->setMainWidget( widget );
	
	connect( _configDialog, SIGNAL( finished() ), this, SLOT( slotDialogClosed() ) );
	connect( _configDialog, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );
	
	_configDialog->show();
}

void KornShell::readConfig()
{
	static bool shownConfig = false;
	
	switch( _const_settings->layout() )
	{
	case Settings::Horizontal:
		_box = new HVContainer( Qt::Horizontal, this );
		break;
	case Settings::Vertical:
		_box = new HVContainer( Qt::Vertical, this );
		break;
	case Settings::Docked:
		_box = new DockedContainer( this );
		break;
	}
	
	connect( _box, SIGNAL( showConfiguration() ), this, SLOT( optionDlg() ) );
			
	_box->readConfig( _const_settings, _settings );

	//Show configuration dialog of no boxes are configurated
	if( !_const_settings->getBox( 0 ) )
		//If user pressed cancel, or did not add a box, close KOrn
		if( !shownConfig )
		{
			shownConfig = true;
			optionDlg();
		}
		else
		{
			kapp->quit();
		}
}

void KornShell::slotDialogClosed()
{
	_configDialog->deleteLater(); _configDialog = 0;

	//At this time, just delete all widgets and make a new one.
	//Maybe, this should replaces later by a better variant.
	slotApply();
}

void KornShell::slotApply()
{
	//At this time, just delete all widgets and make a new one.
	//Maybe, this should replaces later by a better variant.
	
	delete _box; _box = 0;

	readConfig();
	_box->showBox();
}

#include "kornshell.moc"
