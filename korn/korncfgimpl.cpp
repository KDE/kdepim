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
 
#include "korncfgimpl.h"

#include "kornboxcfgimpl.h"
#include "password.h"
#include "settings.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmenu.h>

#include <QCheckBox>
#include <QString>


/*
 * parent should be of type KDialog
 */
KornCfgImpl::KornCfgImpl( QWidget * parent )
	: QWidget( parent ),
	Ui_KornCfgWidget(),	
	m_settings( Settings::self() ),
	m_base( 0 )
{
	setupUi( this );

	connect( parent, SIGNAL( okClicked() ), this, SLOT( slotOK() ) );
	connect( parent, SIGNAL( cancelClicked() ), this, SLOT( slotCancel() ) );
	connect( parent, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );

	connect( pbAdd, SIGNAL(clicked()), this, SLOT(slotAdd()) );
	connect( pbRemove, SIGNAL(clicked()), this, SLOT(slotRemove()) );
	connect( pbMoveUp, SIGNAL(clicked()), this, SLOT(slotMoveUp()) );
	connect( pbMoveDown, SIGNAL(clicked()), this, SLOT(slotMoveDown()) );
	connect( pbEdit, SIGNAL(clicked()), this, SLOT(slotEditBox()) );
	connect( lsBoxes, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotEditBox()) );

	lsBoxes->setSelectionMode( QAbstractItemView::SingleSelection );

	readConfig();
}

KornCfgImpl::~KornCfgImpl()
{
}

void KornCfgImpl::slotEditBox()
{
	if( m_base )
		return; //Already a dialog open
	if( lsBoxes->currentRow() < 0 ) //TODO: test if it works
		return; //No item selected
	lsBoxes->setEnabled( false );
	pbAdd->setEnabled( false );
	pbRemove->setEnabled( false );
	pbMoveUp->setEnabled( false );
	pbMoveDown->setEnabled( false );
	
	m_base = new KDialog( this );
	m_base->setCaption( i18n("Box Configuration") );
	m_base->setButtons( KDialog::Ok | KDialog::Cancel );
	m_base->setModal( false );
	m_base->showButtonSeparator( true );
	KornBoxCfgImpl *widget = new KornBoxCfgImpl( m_base, m_settings, m_settings->getBox( lsBoxes->currentRow() ) );
	
	connect( m_base, SIGNAL( finished() ), this, SLOT( slotDialogDestroyed() ) );
	
	m_base->setMainWidget( widget );
	
	m_base->show();
}

void KornCfgImpl::slotDialogDestroyed()
{
	m_base->deleteLater(); m_base = 0;
	lsBoxes->setEnabled( true );
	pbAdd->setEnabled( true );
	pbRemove->setEnabled( true );
	pbMoveUp->setEnabled( true );
	pbMoveDown->setEnabled( true );
	if( m_settings->getBox( lsBoxes->currentRow() ) )
		lsBoxes->currentItem()->setText( m_settings->getBox( lsBoxes->currentRow() )->boxName() );
}

void KornCfgImpl::slotActivated( const QModelIndex& )
{
	slotEditBox();
}

void KornCfgImpl::slotOK()
{
	writeConfig();
}

void KornCfgImpl::slotCancel()
{
	//m_config->rollback();
}

void KornCfgImpl::slotApply()
{
	writeConfig();
}

void KornCfgImpl::readConfig()
{
	int xx = 0;
	BoxSettings *setting;

	m_settings->readConfig();
	
	switch( m_settings->layout() )
	{
	case Settings::Horizontal:
		rbHorizontal->setChecked( true );
		break;
	case Settings::Vertical:
		rbVertical->setChecked( true );
		break;
	case Settings::Docked:
		rbDocked->setChecked( true );
		break;
	}
	chUseWallet->setChecked( m_settings->useWallet() );

	while( ( setting = m_settings->getBox( xx ) ) )
	{
		lsBoxes->addItem( m_settings->getBox( xx )->boxName() );
		++xx;
	}
}

void KornCfgImpl::writeConfig()
{
	if( rbHorizontal->isChecked() )
		m_settings->setLayout( Settings::Horizontal );
	if( rbVertical->isChecked() )
		m_settings->setLayout( Settings::Vertical );
	if( rbDocked->isChecked() )
		m_settings->setLayout( Settings::Docked );

	//Default is 'false' here, because if no option is set, KWallet isn't used.
	//if( m_settings->useWallet() != chUseWallet->isChecked() )
		//Configuration box changed => setting over configuration
		//rewritePasswords();

	m_settings->setUseWallet( chUseWallet->isChecked() );
	m_settings->writeConfig();
}

void KornCfgImpl::slotAdd()
{
	m_settings->addBox();
	lsBoxes->addItem( "new box" );
	lsBoxes->setCurrentRow( lsBoxes->count() - 1 );
	slotEditBox();
}

void KornCfgImpl::slotRemove()
{
	if( lsBoxes->currentRow() < 0 )
		return; // No item selected

	//Move item to behind
	while( lsBoxes->currentRow() != lsBoxes->count() - 1 )
		slotMoveDown();

	//Delete box and remove from list
	m_settings->deleteBox( lsBoxes->count() - 1 );
	lsBoxes->takeItem( lsBoxes->count() - 1 );
}

void KornCfgImpl::slotMoveUp()
{
	if( lsBoxes->currentRow() < 1 )
		return; //Already first, or there is no item selected
	m_settings->swapBox( lsBoxes->currentRow() - 1, lsBoxes->currentRow() );
	if( !m_settings->getBox( lsBoxes->currentRow() ) )
		kWarning() <<"The settings do not match with the list widget";
	else
		lsBoxes->currentItem()->setText( m_settings->getBox( lsBoxes->currentRow() )->boxName() );
	lsBoxes->setCurrentRow( lsBoxes->currentRow() - 1 );
	if( !m_settings->getBox( lsBoxes->currentRow() ) )
		kWarning() <<"The settings do not match with the list widget";
	else
		lsBoxes->currentItem()->setText( m_settings->getBox( lsBoxes->currentRow() )->boxName() );
}

void KornCfgImpl::slotMoveDown()
{
	if( lsBoxes->currentRow() < 0 || lsBoxes->currentRow() + 1 >= lsBoxes->count() )
		return; //Already last, or there is no item selected
	m_settings->swapBox( lsBoxes->currentRow(), lsBoxes->currentRow() + 1 );
	if( !m_settings->getBox( lsBoxes->currentRow() ) )
		kWarning() <<"The settings do not match with the list widget";
	else
		lsBoxes->currentItem()->setText( m_settings->getBox( lsBoxes->currentRow() )->boxName() );
	lsBoxes->setCurrentRow( lsBoxes->currentRow() + 1 );
	if( !m_settings->getBox( lsBoxes->currentRow() ) )
		kWarning() <<"The settings do not match with the list widget";
	else
		lsBoxes->currentItem()->setText( m_settings->getBox( lsBoxes->currentRow() )->boxName() );
}

/*void KornCfgImpl::rewritePasswords()
{
	int box = 0 - 1;
	int account = 0 - 1;
	KConfigGroup *group;

	while( m_config->hasGroup( QString( "korn-%1" ).arg( ++box ) ) )
	{
		account = 0 - 1;
		while( m_config->hasGroup( QString( "korn-%1-%2" ).arg( box ).arg( ++account ) ) )
		{
			group = new KConfigGroup( m_config, QString( "korn-%1-%2" ).arg( box ).arg( account ) );
			KOrnPassword::rewritePassword( box, account, *group, chUseWallet->isChecked() );
			delete group;
		}
	}
	
	m_config->setGroup( "korn" );
}*/

#include "korncfgimpl.moc"
