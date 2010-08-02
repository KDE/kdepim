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

#include "keditlistboxman.h"
#include "kornaccountcfgimpl.h"
#include "password.h"

#include <kconfig.h>
#include <kcolorbutton.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kfontdialog.h>
#include <klocale.h>
#include <kicondialog.h>
#include <kurlrequester.h>

#include <tqcheckbox.h>
#include <tqcolor.h>
#include <tqfont.h>
#include <tqlabel.h>
#include <tqstring.h>

KornBoxCfgImpl::KornBoxCfgImpl( TQWidget * parent, const char * name )
	: KornBoxCfg( parent, name ),
	_config( 0 ),
	_base( 0 ),
	_index( -1 )
{
	_fonts[ 0 ] = new TQFont;
	_fonts[ 1 ] = new TQFont;
	_anims[ 0 ] = new TQString;
	_anims[ 1 ] = new TQString;

	lbLeft->setText( i18n( "Left mousebutton", "Left" ) );
	if( lbLeft->text() == "Left" )
		lbLeft->setText( i18n( "Left" ) );
	lbRight->setText( i18n( "Right mousebutton", "Right" ) );
	if( lbRight->text() == "Right" )
		lbRight->setText( i18n( "Right" ) );
	
	connect( parent, TQT_SIGNAL( okClicked() ), this, TQT_SLOT( slotOK() ) );
	connect( parent, TQT_SIGNAL( cancelClicked() ), this, TQT_SLOT( slotCancel() ) );
	
	elbAccounts->setTitle( i18n( "Accounts" ) );

	connect( elbAccounts, TQT_SIGNAL( elementsSwapped( int, int ) ), this, TQT_SLOT( slotAccountsSwapped( int, int ) ) );
	connect( elbAccounts, TQT_SIGNAL( elementDeleted( int ) ), this, TQT_SLOT( slotAccountDeleted( int ) ) );
}

KornBoxCfgImpl::~KornBoxCfgImpl()
{
	delete _fonts[ 0 ];
	delete _fonts[ 1 ];
	delete _anims[ 0 ];
	delete _anims[ 1 ];
}

void KornBoxCfgImpl::readConfig( KConfig * config, const int index )
{
	_config = config;
	_index = index;
	
	_config->setGroup( TQString( "korn-%1" ).arg( index ) );

	readViewConfig();
	readEventConfig();
	readDCOPConfig();
	readAccountsConfig();
}

void KornBoxCfgImpl::writeConfig( KConfig * config, const int index )
{
	config->setGroup( TQString( "korn-%1" ).arg( index ) );
	
	writeViewConfig( config );
	writeEventConfig( config );
	writeDCOPConfig( config );
	writeAccountsConfig( config );
}

//private
void KornBoxCfgImpl::readViewConfig()
{
	this->chNormalText->setChecked(_config->readBoolEntry ( "hasnormalfgcolour", true ) );
	this->cbNormalText->setColor(  _config->readColorEntry( "normalfgcolour", &Qt::black ) );
	this->chNewText->setChecked(   _config->readBoolEntry ( "hasnewfgcolour", true ) );
	this->cbNewText->setColor(     _config->readColorEntry( "newfgcolour", &Qt::black ) );
	this->chNormalBack->setChecked(_config->readBoolEntry ( "hasnormalbgcolour", false ) );
	this->cbNormalBack->setColor(  _config->readColorEntry( "normalbgcolour", &Qt::white ) );
	this->chNewBack->setChecked(   _config->readBoolEntry ( "hasnewbgcolour", false ) );
	this->cbNewBack->setColor(     _config->readColorEntry( "newbgcolour", &Qt::white ) );
	
	this->chNormalIcon->setChecked(_config->readBoolEntry( "hasnormalicon", false ) );
	this->ibNormalIcon->setIcon(   _config->readEntry    ( "normalicon", "" ) );
	this->chNewIcon->setChecked(   _config->readBoolEntry( "hasnewicon", false ) );
	this->ibNewIcon->setIcon(      _config->readEntry    ( "newicon", "" ) );
	
	this->chNormalFont->setChecked(_config->readBoolEntry( "hasnormalfont", false ) );
	this->chNewFont->setChecked   (_config->readBoolEntry( "hasnewfont", false ) );
	this->chNormalAnim->setChecked(_config->readBoolEntry( "hasnormalanim", false ) );
	this->chNewAnim->setChecked(   _config->readBoolEntry( "hasnewanim", false ) );
	*_fonts[ 0 ] = _config->readFontEntry( "normalfont" );
	*_fonts[ 1 ] = _config->readFontEntry( "newfont" );
	*_anims[ 0 ] = _config->readEntry    ( "normalanim", "" );
	*_anims[ 1 ] = _config->readEntry    ( "newanim", "" );
}

void KornBoxCfgImpl::readEventConfig()
{
	this->chLeftRecheck  ->setChecked( _config->readBoolEntry( "leftrecheck", true ) );
	this->chMiddleRecheck->setChecked( _config->readBoolEntry( "middlerecheck", false ) );
	this->chRightRecheck ->setChecked( _config->readBoolEntry( "rightrecheck", false ) );
	
	this->chLeftReset  ->setChecked( _config->readBoolEntry( "leftreset", false ) );
	this->chMiddleReset->setChecked( _config->readBoolEntry( "middlereset", false ) );
	this->chRightReset ->setChecked( _config->readBoolEntry( "rightreset", false ) );
	
	this->chLeftView  ->setChecked( _config->readBoolEntry( "leftview", false ) );
	this->chMiddleView->setChecked( _config->readBoolEntry( "middleview", false ) );
	this->chRightView ->setChecked( _config->readBoolEntry( "rightview", false ) );
	
	this->chLeftRun  ->setChecked( _config->readBoolEntry( "leftrun", false ) );
	this->chMiddleRun->setChecked( _config->readBoolEntry( "middlerun", false ) );
	this->chRightRun ->setChecked( _config->readBoolEntry( "rightrun", false ) );
	
	this->chLeftPopup  ->setChecked( _config->readBoolEntry( "leftpopup", false ) );
	this->chMiddlePopup->setChecked( _config->readBoolEntry( "middlepopup", false ) );
	this->chRightPopup ->setChecked( _config->readBoolEntry( "rightpopup", true ) );
	
	this->edCommand->setURL( _config->readEntry( "command", "" ) );
	
	this->edNewRun->setURL( _config->readEntry( "newcommand", "" ) );
	this->edPlaySound->setURL( _config->readEntry( "sound", "" ) );
	this->chShowPassive->setChecked( _config->readBoolEntry( "passivepopup", false ) );
	this->chPassiveDate->setChecked( _config->readBoolEntry( "passivedate", false ) );
}

void KornBoxCfgImpl::readAccountsConfig()
{
	elbAccounts->setGroupName( TQString( "korn-%1-%2" ).arg( _index ) );
	elbAccounts->setConfig( _config );
}
	
void KornBoxCfgImpl::readDCOPConfig()
{
	elbDCOP->clear();
	elbDCOP->insertStringList( _config->readListEntry( "dcop", ',' ) );
}
	
void KornBoxCfgImpl::writeViewConfig( KConfig* config )
{
	TQColor invalid;
	
	config->writeEntry( "hasnormalfgcolour", this->chNormalText->isChecked() );
	config->writeEntry( "normalfgcolour",    this->chNormalText->isChecked() ? this->cbNormalText->color() : invalid );
	config->writeEntry( "hasnewfgcolour",    this->chNewText->isChecked() );
	config->writeEntry( "newfgcolour",       this->chNewText->isChecked()    ? this->cbNewText->color() : invalid );
	config->writeEntry( "hasnormalbgcolour", this->chNormalBack->isChecked() );
	config->writeEntry( "normalbgcolour",    this->chNormalBack->isChecked() ? this->cbNormalBack->color() : invalid );
	config->writeEntry( "hasnewbgcolour",    this->chNewBack->isChecked() );
	config->writeEntry( "newbgcolour",       this->chNewBack->isChecked()    ? this->cbNewBack->color() : invalid );
	
	config->writeEntry( "hasnormalicon", this->chNormalIcon->isChecked() );
	config->writeEntry( "normalicon",    this->chNormalIcon->isChecked() ? this->ibNormalIcon->icon() : "" );
	config->writeEntry( "hasnewicon",    this->chNewIcon->isChecked() );
	config->writeEntry( "newicon",       this->chNewIcon->isChecked()    ? this->ibNewIcon->icon() : "" );
	
	config->writeEntry( "hasnormalfont", this->chNormalFont->isChecked() );
	config->writeEntry( "normalfont",    this->chNormalFont->isChecked() ? *_fonts[ 0 ] : TQFont() );
	config->writeEntry( "hasnewfont",    this->chNewFont->isChecked() );
	config->writeEntry( "newfont",       this->chNewFont->isChecked() ? *_fonts[ 1 ] : TQFont() );
	config->writeEntry( "hasnormalanim", this->chNormalAnim->isChecked() );
	config->writeEntry( "normalanim",    this->chNormalAnim->isChecked() ? *_anims[ 0 ] : "" );
	config->writeEntry( "hasnewanim",    this->chNewAnim->isChecked() );
	config->writeEntry( "newanim",       this->chNewAnim->isChecked() ? *_anims[ 1 ] : "" );
	
}

void KornBoxCfgImpl::writeEventConfig( KConfig *config )
{
	config->writeEntry( "leftrecheck",   this->chLeftRecheck  ->isChecked() );
	config->writeEntry( "middlerecheck", this->chMiddleRecheck->isChecked() );
	config->writeEntry( "rightrecheck",  this->chRightRecheck ->isChecked() );
	
	config->writeEntry( "leftreset",   this->chLeftReset  ->isChecked() );
	config->writeEntry( "middlereset", this->chMiddleReset->isChecked() );
	config->writeEntry( "rightreset",  this->chRightReset ->isChecked() );
	
	config->writeEntry( "leftview",   this->chLeftView  ->isChecked() );
	config->writeEntry( "middleview", this->chMiddleView->isChecked() );
	config->writeEntry( "rightview",  this->chRightView ->isChecked() );
	
	config->writeEntry( "leftrun",   this->chLeftRun  ->isChecked()  );
	config->writeEntry( "middlerun", this->chMiddleRun->isChecked()  );
	config->writeEntry( "rightrun",  this->chRightRun ->isChecked() );
	
	config->writeEntry( "leftpopup",   this->chLeftPopup  ->isChecked() );
	config->writeEntry( "middlepopup", this->chMiddlePopup->isChecked() );
	config->writeEntry( "rightpopup",  this->chRightPopup ->isChecked() );
	
	config->writeEntry( "command", this->edCommand->url() );
	
	config->writeEntry( "newcommand", this->edNewRun->url() );
	config->writeEntry( "sound", this->edPlaySound->url() );
	config->writeEntry( "passivepopup", this->chShowPassive->isChecked() );
	config->writeEntry( "passivedate", this->chPassiveDate->isChecked() );
}

void KornBoxCfgImpl::writeAccountsConfig( KConfig */*config */)
{
}

void KornBoxCfgImpl::writeDCOPConfig( KConfig *config )
{
	config->writeEntry( "dcop", elbDCOP->items(), ',' );
}

void KornBoxCfgImpl::slotEditBox()
{
	if( _base )
		return; //Already a dialog open
	if( elbAccounts->listBox()->currentItem() < 0 )
		return; //No item selected
	elbAccounts->setEnabled( false );
	
	_base = new KDialogBase( this, "Account Dialog", false, i18n("Box Configuration"),
					     KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );
	KornAccountCfgImpl *widget = new KornAccountCfgImpl( _base, "Account Widget" );

	_base->setMainWidget( widget );
	
	connect( _base, TQT_SIGNAL( finished() ), this, TQT_SLOT( slotDialogDestroyed() ) );

	_group = new KConfigGroup( _config, TQString( "korn-%1-%2" ).
			arg( _index ).arg(elbAccounts->listBox()->currentItem() ) );
	
	TQMap< TQString, TQString > *map = new TQMap< TQString, TQString >( _config->entryMap( TQString( "korn-%1-%2" ).
			                        arg( _index ).arg(elbAccounts->listBox()->currentItem() ) ) );
	widget->readConfig( _group, map, _index, elbAccounts->listBox()->currentItem() );
	delete map;

	_base->show();
}
	
void KornBoxCfgImpl::slotActivated( const TQString& )
{
	slotEditBox();
}

void KornBoxCfgImpl::slotActivated( const int )
{
	slotEditBox();
}

void KornBoxCfgImpl::slotSetDefaults( const TQString& name, const int, KConfig* config )
{
	config->writeEntry( "name", name );
	config->writeEntry( "protocol", "mbox" );
	config->writeEntry( "host", TQString::null );
	config->writeEntry( "port", TQString::null );
	config->writeEntry( "username", TQString::null );
	config->writeEntry( "mailbox", "/var/spool/mail/" );
	config->writeEntry( "savepassword", 0 );
	config->writeEntry( "password", TQString::null );
	config->writeEntry( "auth", TQString::null );
	config->writeEntry( "interval", 300 );
	config->writeEntry( "boxsettings", true );
	config->writeEntry( "command", "" );
	config->writeEntry( "sound", "" );
	config->writeEntry( "passivepopup", false );
	config->writeEntry( "passivedate", false );
}

void KornBoxCfgImpl::slotChangeNormalAnim()
{
	*_anims[ 0 ] = KFileDialog::getOpenFileName( *_anims[ 0 ], ".mng .gif", this, i18n("Normal animation") );
}

void KornBoxCfgImpl::slotChangeNewAnim()
{
	*_anims[ 1 ] = KFileDialog::getOpenFileName( *_anims[ 1 ], ".mng .gif", this, i18n("Normal animation") );
}

void KornBoxCfgImpl::slotChangeNormalFont()
{
	KFontDialog fd( this, "font dialog" );
	fd.setFont( *_fonts[ 0 ], false );
	fd.exec();
	*_fonts[ 0 ] = fd.font();
}

void KornBoxCfgImpl::slotChangeNewFont()
{
	KFontDialog fd( this, "font dialog" );
	fd.setFont( *_fonts[ 1 ], false );
	fd.exec();
	*_fonts[ 1 ] = fd.font();
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
	writeConfig( _config, _index );
}

void KornBoxCfgImpl::slotCancel()
{
	readConfig( _config, _index );
}

void KornBoxCfgImpl::slotDialogDestroyed()
{
	_base->deleteLater(); _base = 0;
	_group->deleteLater();
	elbAccounts->setEnabled( true );
}

void KornBoxCfgImpl::slotAccountsSwapped( int account1, int account2 )
{
	KOrnPassword::swapKOrnWalletPassword( _index, account1, _index, account2 );
}

void KornBoxCfgImpl::slotAccountDeleted( int account )
{
	KOrnPassword::deleteKOrnPassword( _index, account );
}

#include "kornboxcfgimpl.moc"

