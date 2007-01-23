/*
 * Copyright (C) 2004-2006, Mart Kelder (mart@kelder31.nl)
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

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>

#include <qtimer.h>

KornShell::KornShell( QWidget * parent, const char * name )
	: QWidget( parent, name ),
	_config( new KConfig( "kornrc" ) ),
	_box( 0 ),
	_configDialog( 0 ),
	_show( false )
{
	_config->checkUpdate( "korn_kde_3_4_config_change", "korn-3-4-config_change.upd" );
	if( kapp->isRestored() )
	{
		_config->setGroup( "korn" );
		QTimer::singleShot( _config->readNumEntry( "session_startup_delay", 2000 ), this, SLOT(readConfig()) );
		kdDebug() << "startup delayed" << endl;
	}
	else
	{
		kdDebug() << "Startup not delayed" << endl;
		readConfig();
	}

}

KornShell::~KornShell()
{
	if( _box )
		_box->writeConfig( _config );
	delete _config;
	delete _box;
}

void KornShell::show()
{
	if( _box )
		_box->showBox();
	else
		_show = true;
}

void KornShell::optionDlg()
{
	if( _configDialog )
	{
		_configDialog->show();
		return;
	}
	
	_configDialog = new KDialogBase( 0, "Configuration Dialog", false, i18n( "Korn Configuration" ),
					 KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Apply, KDialogBase::Ok, true );
	
	KornCfgImpl *widget = new KornCfgImpl( _configDialog, "Configuration widget" );
	_configDialog->setMainWidget( widget );
	
	connect( _configDialog, SIGNAL( finished() ), this, SLOT( slotDialogClosed() ) );
	connect( _configDialog, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );
	
	_configDialog->show();
}

void KornShell::readConfig()
{
	static bool shownConfig = false;
	
	_config->setGroup( "korn" );
	QChar layout = _config->readEntry( "layout", "Docked" )[0].upper();
	KOrnPassword::setUseWallet( _config->readBoolEntry( "usewallet", false ) );
	
	if( layout == 'H' )
		_box = new HVContainer( Qt::Horizontal, this, "horizontal container" );
	else if( layout == 'V' )
		_box = new HVContainer( Qt::Vertical, this, "vertical container" );
	else
		_box = new DockedContainer( this, "docked container" );

	connect( _box, SIGNAL( showConfiguration() ), this, SLOT( optionDlg() ) );
			
	_box->readConfig( _config );

	//Show configuration dialog of no boxes are configurated
	if( !_config->hasGroup( "korn-0" ) )
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

	if( _show )
	{
		// Show is called, but _box wasn't available at that time
		show();
		_show = false;
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

	_config->reparseConfiguration();
	
	readConfig();
	_box->showBox();
}

#include "kornshell.moc"
