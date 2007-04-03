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
 
class KConfig;
#include "kornboxcfgimpl.h"

#include "kornaccountcfgimpl.h"
#include "password.h"
#include "settings.h"

#include <kconfig.h>
#include <kcolorbutton.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kfontdialog.h>
#include <klocale.h>
#include <kicondialog.h>
#include <kurlrequester.h>

#include <QCheckBox>
#include <QColor>
#include <QFont>
#include <QLabel>
#include <QListView>
#include <QString>

KornBoxCfgImpl::KornBoxCfgImpl( QWidget * parent, Settings *glob_settings, BoxSettings *settings )
	: QWidget( parent ),
	Ui_KornBoxCfg(),
	m_glob_settings( glob_settings ),
	m_settings( settings ),
	m_base( 0 ),
	m_index( -1 )
{
	setupUi( this );
	
	m_fonts[ 0 ] = new QFont;
	m_fonts[ 1 ] = new QFont;
	m_anims[ 0 ] = new QString;
	m_anims[ 1 ] = new QString;

	lbLeft->setText( i18nc( "Left mousebutton", "Left" ) );
	if( lbLeft->text() == "Left" )
		lbLeft->setText( i18n( "Left" ) );
	lbRight->setText( i18nc( "Right mousebutton", "Right" ) );
	if( lbRight->text() == "Right" )
		lbRight->setText( i18n( "Right" ) );

	connect( parent, SIGNAL( okClicked() ), this, SLOT( slotOK() ) );
	connect( parent, SIGNAL( cancelClicked() ), this, SLOT( slotCancel() ) );
	
	connect( chNormalText, SIGNAL(toggled(bool)), cbNormalText, SLOT(setEnabled(bool)) );
	connect( chNewText, SIGNAL(toggled(bool)), cbNewText, SLOT(setEnabled(bool)) );
	connect( chNormalBack, SIGNAL(toggled(bool)), cbNormalBack, SLOT(setEnabled(bool)) );
	connect( chNewBack, SIGNAL(toggled(bool)), cbNewBack, SLOT(setEnabled(bool)) );
	connect( chNormalIcon, SIGNAL(toggled(bool)), ibNormalIcon, SLOT(setEnabled(bool)) );
	connect( chNewIcon, SIGNAL(toggled(bool)), ibNewIcon, SLOT(setEnabled(bool)) );
	connect( chShowPassive, SIGNAL(toggled(bool)), chPassiveDate, SLOT(setEnabled(bool)) );
	connect( pbAdd, SIGNAL(clicked()), this, SLOT(slotAddAccount()) );
	connect( pbRemove, SIGNAL(clicked()), this, SLOT(slotRemoveAccount()) );
	connect( pbMoveUp, SIGNAL(clicked()), this, SLOT(slotMoveUp()) );
	connect( pbMoveDown, SIGNAL(clicked()), this, SLOT(slotMoveDown()) );
	connect( pbEdit, SIGNAL(clicked()), this, SLOT(slotEditBox()) );
	connect( lsAccounts, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotEditBox()) );
	
	connect( pbNormalFont, SIGNAL(pressed()), this, SLOT(slotChangeNormalFont()) );
	connect( pbNewFont, SIGNAL(pressed()), this, SLOT(slotChangeNewFont()) );
	connect( pbNormalAnim, SIGNAL(released()), this, SLOT(slotChangeNormalAnim()) );
	connect( pbNewAnim, SIGNAL(pressed()), this, SLOT(slotChangeNewAnim()) );
	connect( chNormalFont, SIGNAL(toggled(bool)), pbNormalFont, SLOT(setEnabled(bool)) );
	connect( chNewFont, SIGNAL(toggled(bool)), pbNewFont, SLOT(setEnabled(bool)) );
	connect( chNormalAnim, SIGNAL(toggled(bool)), pbNormalAnim, SLOT(setEnabled(bool)) );
	connect( chNewAnim, SIGNAL(toggled(bool)), pbNewAnim, SLOT(setEnabled(bool)) );
	connect( chNormalAnim, SIGNAL(toggled(bool)), this, SLOT(slotNormalAnimToggled(bool)) );
	connect( chNewAnim, SIGNAL(toggled(bool)), this, SLOT(slotNewAnimToggled(bool)) );

	this->edName->setText( m_settings->boxName() );
	readViewConfig();
	readEventConfig();
	readAccountsConfig();
}

KornBoxCfgImpl::~KornBoxCfgImpl()
{
	delete m_fonts[ 0 ];
	delete m_fonts[ 1 ];
	delete m_anims[ 0 ];
	delete m_anims[ 1 ];
}

void KornBoxCfgImpl::writeConfig()
{
	m_settings->setBoxName( this->edName->text() );
	writeViewConfig();
	writeEventConfig();
}

//private
void KornBoxCfgImpl::readViewConfig()
{
	this->chNormalText->setChecked( m_settings->hasForegroundColor( BoxSettings::Normal ) );
	this->cbNormalText->setColor(   m_settings->foregroundColor( BoxSettings::Normal ) );
	this->chNewText->setChecked(    m_settings->hasForegroundColor( BoxSettings::New ) );
	this->cbNewText->setColor(      m_settings->foregroundColor( BoxSettings::New ) );
	this->chNormalBack->setChecked( m_settings->hasBackgroundColor( BoxSettings::Normal ) );
	this->cbNormalBack->setColor(   m_settings->backgroundColor( BoxSettings::Normal ) );
	this->chNewBack->setChecked(    m_settings->hasBackgroundColor( BoxSettings::New ) );
	this->cbNewBack->setColor(      m_settings->backgroundColor( BoxSettings::New ) );
	
	this->chNormalIcon->setChecked( m_settings->hasIcon( BoxSettings::Normal ) );
	this->ibNormalIcon->setIcon(    m_settings->icon( BoxSettings::Normal ) );
	this->chNewIcon->setChecked(    m_settings->hasIcon( BoxSettings::New ) );
	this->ibNewIcon->setIcon(       m_settings->icon( BoxSettings::New ) );
	
	this->chNormalFont->setChecked( m_settings->hasFont( BoxSettings::Normal ) );
	this->chNewFont->setChecked   ( m_settings->hasFont( BoxSettings::New ) );
	this->chNormalAnim->setChecked( m_settings->hasAnimation( BoxSettings::Normal ) );
	this->chNewAnim->setChecked(    m_settings->hasAnimation( BoxSettings::New ) );
	*m_fonts[ 0 ] = m_settings->font( BoxSettings::Normal );
	*m_fonts[ 1 ] = m_settings->font( BoxSettings::New );
	*m_anims[ 0 ] = m_settings->animation( BoxSettings::Normal );
	*m_anims[ 1 ] = m_settings->animation( BoxSettings::New );
}

void KornBoxCfgImpl::readEventConfig()
{
	this->chLeftRecheck  ->setChecked( m_settings->clickCommand( Qt::LeftButton,  BoxSettings::Recheck ) );
	this->chMiddleRecheck->setChecked( m_settings->clickCommand( Qt::MidButton,   BoxSettings::Recheck ) );
	this->chRightRecheck ->setChecked( m_settings->clickCommand( Qt::RightButton, BoxSettings::Recheck ) );
	
	this->chLeftReset  ->setChecked( m_settings->clickCommand( Qt::LeftButton,  BoxSettings::Reset ) );
	this->chMiddleReset->setChecked( m_settings->clickCommand( Qt::MidButton,   BoxSettings::Reset ) );
	this->chRightReset ->setChecked( m_settings->clickCommand( Qt::RightButton, BoxSettings::Reset ) );
	
	this->chLeftView  ->setChecked( m_settings->clickCommand( Qt::LeftButton,  BoxSettings::View ) );
	this->chMiddleView->setChecked( m_settings->clickCommand( Qt::MidButton,   BoxSettings::View ) );
	this->chRightView ->setChecked( m_settings->clickCommand( Qt::RightButton, BoxSettings::View ) );
	
	this->chLeftRun  ->setChecked( m_settings->clickCommand( Qt::LeftButton,  BoxSettings::Run ) );
	this->chMiddleRun->setChecked( m_settings->clickCommand( Qt::MidButton,   BoxSettings::Run ) );
	this->chRightRun ->setChecked( m_settings->clickCommand( Qt::RightButton, BoxSettings::Run ) );
	
	this->chLeftPopup  ->setChecked( m_settings->clickCommand( Qt::LeftButton,  BoxSettings::Popup ) );
	this->chMiddlePopup->setChecked( m_settings->clickCommand( Qt::MidButton,   BoxSettings::Popup ) );
	this->chRightPopup ->setChecked( m_settings->clickCommand( Qt::RightButton, BoxSettings::Popup ) );
	
	this->edCommand->setUrl( m_settings->command() );
	
	this->edNewRun->setUrl( m_settings->newCommand() );
	this->edPlaySound->setUrl( m_settings->sound() );
	this->chShowPassive->setChecked( m_settings->passivePopup() );
	this->chPassiveDate->setChecked( m_settings->passiveDate() );
}

void KornBoxCfgImpl::readAccountsConfig()
{
	AccountSettings *setting;
	int xx = 0;

	while( ( setting = m_settings->account( xx ) ) )
	{
		lsAccounts->addItem( setting->accountName() );
		++xx;
	}
}
	
void KornBoxCfgImpl::writeViewConfig()
{
	QColor invalid;
	
	m_settings->setHasForegroundColor( BoxSettings::Normal, this->chNormalText->isChecked() );
	m_settings->setForegroundColor( BoxSettings::Normal, this->chNormalText->isChecked() ? this->cbNormalText->color() : invalid );
	m_settings->setHasForegroundColor( BoxSettings::New, this->chNewText->isChecked() );
	m_settings->setForegroundColor( BoxSettings::New, this->chNewText->isChecked() ? this->cbNewText->color() : invalid );
	m_settings->setHasBackgroundColor( BoxSettings::Normal, this->chNormalBack->isChecked() );
	m_settings->setBackgroundColor( BoxSettings::Normal, this->chNormalBack->isChecked() ? this->cbNormalBack->color() : invalid );
	m_settings->setHasBackgroundColor( BoxSettings::New, this->chNewBack->isChecked() );
	m_settings->setBackgroundColor( BoxSettings::New, this->chNewBack->isChecked() ? this->cbNewBack->color() : invalid );
	
	m_settings->setHasIcon( BoxSettings::Normal, this->chNormalIcon->isChecked() );
	m_settings->setIcon( BoxSettings::Normal, this->chNormalIcon->isChecked() ? this->ibNormalIcon->icon() : "" );
	m_settings->setHasIcon( BoxSettings::New, this->chNewIcon->isChecked() );
	m_settings->setIcon( BoxSettings::New, this->chNewIcon->isChecked() ? this->ibNewIcon->icon() : "" );
	
	m_settings->setHasFont( BoxSettings::Normal, this->chNormalFont->isChecked() );
	m_settings->setFont( BoxSettings::Normal, this->chNormalFont->isChecked() ? *m_fonts[ 0 ] : QFont() );
	m_settings->setHasFont( BoxSettings::New, this->chNewFont->isChecked() );
	m_settings->setFont( BoxSettings::New, this->chNewFont->isChecked() ? *m_fonts[ 1 ] : QFont() );
	m_settings->setHasAnimation( BoxSettings::Normal, this->chNormalAnim->isChecked() );
	m_settings->setAnimation( BoxSettings::Normal, this->chNormalAnim->isChecked() ? *m_anims[ 0 ] : "" );
	m_settings->setHasAnimation( BoxSettings::New, this->chNewAnim->isChecked() );
	m_settings->setAnimation( BoxSettings::New, this->chNewAnim->isChecked() ? *m_anims[ 1 ] : "" );
}

void KornBoxCfgImpl::writeEventConfig()
{
	m_settings->setClickCommand( Qt::LeftButton, BoxSettings::Recheck, this->chLeftRecheck->isChecked() );
	m_settings->setClickCommand( Qt::MidButton, BoxSettings::Recheck, this->chMiddleRecheck->isChecked() );
	m_settings->setClickCommand( Qt::RightButton, BoxSettings::Recheck, this->chRightRecheck->isChecked() );
	
	m_settings->setClickCommand( Qt::LeftButton, BoxSettings::Reset, this->chLeftReset->isChecked() );
	m_settings->setClickCommand( Qt::MidButton, BoxSettings::Reset, this->chMiddleReset->isChecked() );
	m_settings->setClickCommand( Qt::RightButton, BoxSettings::Reset, this->chRightReset->isChecked() );

	m_settings->setClickCommand( Qt::LeftButton, BoxSettings::View, this->chLeftView->isChecked() );
	m_settings->setClickCommand( Qt::MidButton, BoxSettings::View, this->chMiddleView->isChecked() );
	m_settings->setClickCommand( Qt::RightButton, BoxSettings::View, this->chRightView->isChecked() );
	
	m_settings->setClickCommand( Qt::LeftButton, BoxSettings::Run, this->chLeftRun->isChecked() );
	m_settings->setClickCommand( Qt::MidButton, BoxSettings::Run, this->chMiddleRun->isChecked() );
	m_settings->setClickCommand( Qt::RightButton, BoxSettings::Run, this->chRightRun->isChecked() );
	
	m_settings->setClickCommand( Qt::LeftButton, BoxSettings::Popup, this->chLeftPopup->isChecked() );
	m_settings->setClickCommand( Qt::MidButton, BoxSettings::Popup, this->chMiddlePopup->isChecked() );
	m_settings->setClickCommand( Qt::RightButton, BoxSettings::Popup, this->chRightPopup->isChecked() );

	m_settings->setCommand( this->edCommand->url().url() );
	
	m_settings->setNewCommand( this->edNewRun->url().url() );
	m_settings->setSound( this->edPlaySound->url().url() );
	m_settings->setPassivePopup( this->chShowPassive->isChecked() );
	m_settings->setPassiveDate( this->chPassiveDate->isChecked() );
}

void KornBoxCfgImpl::slotAddAccount()
{
	m_settings->addAccount();
	lsAccounts->addItem( "new account" );
	lsAccounts->setCurrentRow( lsAccounts->count() - 1 );
	slotEditBox();
}

void KornBoxCfgImpl::slotRemoveAccount()
{
	if( lsAccounts->currentRow() < 0 )
		return;

	while( lsAccounts->currentRow() < lsAccounts->count() - 1 )
		slotMoveDown();
	
	m_settings->deleteAccount( lsAccounts->count() - 1 );
	lsAccounts->takeItem( lsAccounts->count() - 1 );
}

void KornBoxCfgImpl::slotMoveUp()
{
	if( lsAccounts->currentRow() < 1 )
		return; //Already first, or no item selected
	m_settings->swapAccounts( lsAccounts->currentRow() - 1, lsAccounts->currentRow() );
	if( !m_settings->account( lsAccounts->currentRow() ) )
		kWarning() << "The settings do not match with the list widget" << endl;
	else
		lsAccounts->currentItem()->setText( m_settings->account( lsAccounts->currentRow() )->accountName() );
	lsAccounts->setCurrentRow( lsAccounts->currentRow() - 1 );
	if( !m_settings->account( lsAccounts->currentRow() ) )
		kWarning() << "The settings do not match with the list widget" << endl;
	else
		lsAccounts->currentItem()->setText( m_settings->account( lsAccounts->currentRow() )->accountName() );
}

void KornBoxCfgImpl::slotMoveDown()
{
	if( lsAccounts->currentRow() < 0 || lsAccounts->currentRow() < lsAccounts->count() - 1 )
		return; //Already last, or no item selected
	m_settings->swapAccounts( lsAccounts->currentRow(), lsAccounts->currentRow() + 1 );
	if( !m_settings->account( lsAccounts->currentRow() ) )
		kWarning() << "The settings do not match with the list widget" << endl;
	else
		lsAccounts->currentItem()->setText( m_settings->account( lsAccounts->currentRow() )->accountName() );
	lsAccounts->setCurrentRow( lsAccounts->currentRow() + 1 );
	if( !m_settings->account( lsAccounts->currentRow() ) )
		kWarning() << "The settings do not match with the list widget" << endl;
	else
		lsAccounts->currentItem()->setText( m_settings->account( lsAccounts->currentRow() )->accountName() );
}

void KornBoxCfgImpl::slotEditBox()
{
	if( m_base )
		return; //Already a dialog open
	if( lsAccounts->currentRow() < 0 ) //TODO: test this
		return; //No item selected
	if( !m_settings->account( lsAccounts->currentRow() ) )
	{
		kWarning() << "The settings do not match with the list widget" << endl;
		return;
	}

	lsAccounts->setEnabled( false );
	pbAdd->setEnabled( false );
	pbRemove->setEnabled( false );
	pbMoveUp->setEnabled( false );
	pbMoveDown->setEnabled( false );
	pbEdit->setEnabled( false );
	
	m_base = new KDialog( this );
	m_base->setButtons( KDialog::Ok | KDialog::Cancel );
	m_base->setCaption( i18n("Box Configuration") );
	m_base->setModal( false );
	m_base->showButtonSeparator( true );
	KornAccountCfgImpl *widget = new KornAccountCfgImpl( m_base, m_glob_settings, m_settings->account( lsAccounts->currentRow() ) );

	m_base->setMainWidget( widget );
	
	connect( m_base, SIGNAL( finished() ), this, SLOT( slotDialogDestroyed() ) );

	m_base->show();
}

void KornBoxCfgImpl::slotActivated( const QModelIndex& )
{
	slotEditBox();
}
	
void KornBoxCfgImpl::slotChangeNormalAnim()
{
	*m_anims[ 0 ] = KFileDialog::getOpenFileName( *m_anims[ 0 ], ".mng .gif", this, i18n("Normal animation") );
}

void KornBoxCfgImpl::slotChangeNewAnim()
{
	*m_anims[ 1 ] = KFileDialog::getOpenFileName( *m_anims[ 1 ], ".mng .gif", this, i18n("Normal animation") );
}

void KornBoxCfgImpl::slotChangeNormalFont()
{
	KFontDialog fd( this );
	fd.setFont( *m_fonts[ 0 ], false );
	fd.exec();
	*m_fonts[ 0 ] = fd.font();
}

void KornBoxCfgImpl::slotChangeNewFont()
{
	KFontDialog fd( this );
	fd.setFont( *m_fonts[ 1 ], false );
	fd.exec();
	*m_fonts[ 1 ] = fd.font();
}

void KornBoxCfgImpl::slotNormalAnimToggled( bool enabled )
{
	this->chNormalText->setEnabled( !enabled );
	//this->chNormalBack->setEnabled( !enabled );
	this->chNormalIcon->setEnabled( !enabled );
	this->chNormalFont->setEnabled( !enabled );

	this->cbNormalText->setEnabled( !enabled && this->chNormalText->isChecked() );
	//this->cbNormalBack->setEnabled( !enabled && this->chNormalBack->isChecked() );
	this->ibNormalIcon->setEnabled( !enabled && this->chNormalIcon->isChecked() );
	this->pbNormalFont->setEnabled( !enabled && this->chNormalFont->isChecked() );
}

void KornBoxCfgImpl::slotNewAnimToggled( bool enabled )
{
	this->chNewText->setEnabled( !enabled );
	//this->chNewBack->setEnabled( !enabled );
	this->chNewIcon->setEnabled( !enabled );
	this->chNewFont->setEnabled( !enabled );
	
	this->cbNewText->setEnabled( !enabled && this->chNewText->isChecked() );
	//this->cbNewBack->setEnabled( !enabled && this->chNewBack->isChecked() );
	this->ibNewIcon->setEnabled( !enabled && this->chNewIcon->isChecked() );
	this->pbNewFont->setEnabled( !enabled && this->chNewFont->isChecked() );
}

void KornBoxCfgImpl::slotOK()
{
	writeConfig();
}

void KornBoxCfgImpl::slotCancel()
{
	//readConfig( m_config, m_index );
}

void KornBoxCfgImpl::slotDialogDestroyed()
{
	m_base->deleteLater(); m_base = 0;
	lsAccounts->setEnabled( true );
	pbAdd->setEnabled( true );
	pbRemove->setEnabled( true );
	pbMoveUp->setEnabled( true );
	pbMoveDown->setEnabled( true );
	pbEdit->setEnabled( true );
	if( m_settings->account( lsAccounts->currentRow() ) )
		lsAccounts->currentItem()->setText( m_settings->account( lsAccounts->currentRow() )->accountName() );
}

#include "kornboxcfgimpl.moc"

