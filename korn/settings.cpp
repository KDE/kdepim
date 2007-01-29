/*
 * Copyright (C) 2006, Mart Kelder (mart@kelder31.nl)
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


#include "settings.h"

#include <kconfig.h>
#include <kglobal.h>

#include <qstring.h>
#include <qstringlist.h>


KSharedConfig::Ptr* Settings::m_config = 0;
Settings* Settings::m_self = 0;

AccountSettings::AccountSettings( KSharedConfig::Ptr *config, BoxSettings *parent, int boxnr, int accountnr )
	: KConfigSkeleton( *config ),
	m_box( parent ),
	m_config( config ),
	m_boxnr( boxnr ),
	m_accountnr( accountnr ),
	m_protocol( new QString() ),
	m_name( new QString() ),
	m_command( new QString() ),
	m_sound( new QString() ),
	m_settings( new QMap< QString, QString >() )
{
	init();
}

AccountSettings::AccountSettings( const AccountSettings &other )
	: KConfigSkeleton( *other.m_config ),
	m_box( other.m_box ),
	m_config( other.m_config ),
	m_boxnr( other.m_boxnr ),
	m_accountnr( other.m_accountnr ),
	m_protocol( new QString() ),
	m_name( new QString() ),
	m_command( new QString() ),
	m_sound( new QString() ),
	m_settings( new QMap< QString, QString >() )
{
	init();
	copy( other );
}

AccountSettings::~AccountSettings()
{
	delete m_protocol;
	delete m_name;
	delete m_command;
	delete m_sound;
	delete m_settings;
}

AccountSettings& AccountSettings::operator=( const AccountSettings& other )
{
	*m_protocol = *other.m_protocol;
	*m_name = *other.m_name;
	*m_command = *other.m_command;
	*m_sound = *other.m_sound;
	*m_settings = *other.m_settings;

	return *this;
}

void AccountSettings::newGroup( int boxnr, int accountnr )
{
	QString newGroup = QString( "korn-%1-%2" ).arg( boxnr ).arg( accountnr );
	int count = items().count();
	
	for( int xx = 0; xx < count; ++xx )
		items().at( xx )->setGroup( newGroup );
}

QString AccountSettings::protocol() const { return *m_protocol; }
void AccountSettings::setProtocol( const QString& val ) { *m_protocol = val; }

QString AccountSettings::accountName() const { return *m_name; }
void AccountSettings::setAccountName( const QString& val ) { *m_name = val; }

QString AccountSettings::command() const { return m_useBoxSettings ? m_box->command() : *m_command; }
void AccountSettings::setCommand( const QString& val ) { *m_command = val; }

QString AccountSettings::sound() const { return m_useBoxSettings ? m_box->sound() : *m_sound; }
void AccountSettings::setSound( const QString& val ) { *m_sound = val; }

bool AccountSettings::passivePopup() const { return m_useBoxSettings ? m_box->passivePopup() : m_passivePopup; }
bool AccountSettings::passiveDate() const { return m_useBoxSettings ? m_box->passiveDate() : m_passiveDate; }

QMap< QString, QString > AccountSettings::readEntries() const
{
	return *m_settings;
}

void AccountSettings::writeEntries( const QMap< QString, QString > &settings )
{
	*m_settings = settings;
}

void AccountSettings::usrReadConfig()
{
	int xx;
	
	*m_settings = config()->entryMap( currentGroup() );
	
	for( xx = 0; xx < items().count(); ++xx )
		m_settings->remove( items().at( xx )->key() );
}

void AccountSettings::usrWriteConfig()
{
	int xx;
	QMap< QString, QString >::const_iterator it;
	QMap< QString, QString > entries = config()->entryMap( currentGroup() );

	for( xx = 0; xx < items().count(); ++xx )
	{
		m_settings->remove( items().at( xx )->key() );
		entries.remove( items().at( xx )->key() );
	}

	it = entries.constBegin();
	while( it != entries.constEnd() )
	{
		config()->deleteEntry( it.key() );
		++it;
	}
	
	it = m_settings->constBegin();
	while( it != m_settings->constEnd() )
	{
		config()->writeEntry( it.key(), it.value() );
		++it;
	}
}

void AccountSettings::init()
{
	setCurrentGroup( QString( "korn-%1-%2" ).arg( m_boxnr ).arg( m_accountnr ) );
	addItemString( "Protocol", *m_protocol, "mbox", "protocol" );
	addItemString( "Name", *m_name, QString(), "name" );
	addItemBool( "UseBoxSettings", m_useBoxSettings, true, "useboxsettings" );
	addItemInt( "Reset", m_reset, -1, "reset" );
	addItemInt( "Interval", m_interval, 300, "interval" );
	
	addItemBool( "PassivePopup", m_passivePopup, false, "passivepopup" );
	addItemBool( "PassiveDate", m_passiveDate, true, "passivedate" );
	addItemString( "Command", *m_command, QString(), "command" );
	addItemString( "Sound", *m_sound, QString(), "sound" );
}

void AccountSettings::copy( const AccountSettings& other )
{
	m_box = other.m_box;
	m_config = other.m_config;
	*m_protocol = *other.m_protocol;
	*m_name = *other.m_name;
	m_useBoxSettings = other.m_useBoxSettings;
	m_reset = other.m_reset;
	m_interval = other.m_interval;
	m_passivePopup = other.m_passivePopup;
	m_passiveDate = other.m_passiveDate;
	*m_command = *other.m_command;
	*m_sound = *other.m_sound;
	*m_settings = *other.m_settings;
}

BoxSettings::BoxSettings( KSharedConfig::Ptr *config, int boxnr )
	: KConfigSkeleton( *config ),
	m_boxnr( boxnr ),
	m_boxname( new QString() ),
	m_command( new QString() ),
	m_newcommand( new QString() ),
	m_sound( new QString() ),
	m_accounts( new QList< AccountSettings* > ),
	m_config( config )
{
	init();
}

BoxSettings::BoxSettings( const BoxSettings& other )
	: KConfigSkeleton( *other.m_config ),
	m_boxnr( other.m_boxnr ),
	m_boxname( new QString() ),
	m_command( new QString() ),
	m_newcommand( new QString() ),
	m_sound( new QString() ),
	m_accounts( new QList< AccountSettings* > ),
	m_config( other.m_config )
{
	init();
	copy( other );
}

BoxSettings::~BoxSettings()
{
	delete m_boxname;
	delete m_command;
	delete m_newcommand;
	delete m_sound;
	//Delete m_accounts
	while( m_accounts->count() > 0 )
		delete m_accounts->takeLast();
	delete m_accounts;

	for( int xx = 0; xx < 2; ++xx )
	{
		delete m_anim[ xx ];
		delete m_bgColor[ xx ];
		delete m_fgColor[ xx ];
		delete m_font[ xx ];
		delete m_icon[ xx ];
	}
}


void BoxSettings::newGroup( int boxnr )
{
	QString newGroup = QString( "korn-%1" ).arg( boxnr );
	int count = items().count();
	
	for( int xx = 0; xx < count; ++xx )
		items().at( xx )->setGroup( newGroup );

	count = m_accounts->count();

	for( int xx = 0; xx < count; ++xx )
		(*m_accounts)[ xx ]->newGroup( boxnr, xx );

	m_boxnr = boxnr;
}
 
QString BoxSettings::boxName() const { return *m_boxname; }
void BoxSettings::setBoxName( const QString& val ) { *m_boxname = val; }

QString BoxSettings::command() const { return *m_command; }
void BoxSettings::setCommand( const QString& val ) { *m_command = val; }

QString BoxSettings::animation( State state ) const { return *m_anim[ state ]; }
void BoxSettings::setAnimation( State state, const QString& val ) { *m_anim[ state ] = val; }

QColor BoxSettings::backgroundColor( State state ) const { return *m_bgColor[ state ]; }
void BoxSettings::setBackgroundColor( State state, const QColor& val ) { *m_bgColor[ state ] = val; }

QColor BoxSettings::foregroundColor( State state ) const { return *m_fgColor[ state ]; }
void BoxSettings::setForegroundColor( State state, const QColor& val ) { *m_fgColor[ state ] = val; }

QFont BoxSettings::font( State state ) const { return *m_font[ state ]; }
void BoxSettings::setFont( State state, const QFont& val ) { *m_font[ state ] = val; }

QString BoxSettings::icon( State state ) const { return *m_icon[ state ]; }
void BoxSettings::setIcon( State state, const QString& val ) { *m_icon[ state ] = val; }

bool BoxSettings::clickCommand( Qt::MouseButton button, Action action )
{
	if( button & Qt::LeftButton )
		return m_clickCommand[ Left ][ action ];
	else if( button & Qt::MidButton )
		return m_clickCommand[ Middle ][ action ];
	else
		return m_clickCommand[ Right ][ action ];
}

void BoxSettings::setClickCommand( Qt::MouseButton button, Action action, bool val )
{
	if( button & Qt::LeftButton )
		m_clickCommand[ Left ][ action ] = val;
	else if( button & Qt::MidButton )
		m_clickCommand[ Middle ][ action ] = val;
	else
		m_clickCommand[ Right ][ action ] = val;
}

QString BoxSettings::newCommand() const { return *m_newcommand; }
void BoxSettings::setNewCommand( const QString& val ) { *m_newcommand = val; }

QString BoxSettings::sound() const { return *m_sound; }
void BoxSettings::setSound( const QString& val ) { *m_sound = val; }

AccountSettings* BoxSettings::account( int accountnr )
{
	//static AccountSettings dummyAccount( m_boxnr, -1 );
	if( 0 <= accountnr && accountnr < m_accounts->count() )
		return (*m_accounts)[ accountnr ];
	else
		return 0;
}

void BoxSettings::addAccount()
{
	m_accounts->append( new AccountSettings( m_config, this, m_boxnr, m_accounts->count() ) );
}

void BoxSettings::deleteAccount( int accountnr )
{
	while( accountnr + 1 < m_accounts->count() )
	{
		swapAccounts( accountnr, accountnr + 1 );
		++accountnr;
	}
	m_accounts->removeLast();
}

void BoxSettings::swapAccounts( int account1, int account2 )
{
	if( account1 > m_accounts->count() || account2 > m_accounts->count() || account1 < 0 || account2 < 0 || account1 == account2 )
		return;
	(*m_accounts)[ account1 ]->newGroup( m_boxnr, account2 );
	(*m_accounts)[ account2 ]->newGroup( m_boxnr, account1 );
	m_accounts->swap( account1, account2 );
}

void BoxSettings::usrReadConfig()
{
	int accountnr = 0;
	AccountSettings *setting;
	
	while( config()->hasGroup( QString( "korn-%1-%2" ).arg( m_boxnr ).arg( accountnr ) ) )
		++accountnr;

	while( accountnr > m_accounts->count() )
	{
		setting = new AccountSettings( m_config, this, m_boxnr, m_accounts->count() );
		m_accounts->append( setting );
		setting->readConfig();
	}
	while( accountnr < m_accounts->count() )
		delete m_accounts->takeLast();
}

void BoxSettings::usrWriteConfig()
{
	int count = m_accounts->count();
	int xx;

	for( xx = 0; xx < count; ++xx )
		(*m_accounts)[ xx ]->writeConfig();

	xx = count;

	while( config()->hasGroup( QString( "korn-%1-%2" ).arg( m_boxnr ).arg( xx ) ) )
		config()->deleteGroup( QString( "korn-%1-%2" ).arg( m_boxnr ).arg( xx++ ) );
}

void BoxSettings::init()
{
	//Init fields
	for( int xx = 0; xx < 2; ++xx )
	{
		m_anim[ xx ] = new QString();
		m_bgColor[ xx ] = new QColor();
		m_fgColor[ xx ] = new QColor();
		m_font[ xx ] = new QFont;
		m_icon[ xx ] = new QString;
	}

	//Additems
	setCurrentGroup( QString( "korn-%1" ).arg( m_boxnr ) );
	addItemString( "Boxname", *m_boxname, "", "name" );
	addItemString( "Command", *m_command, "", "command" );
	
	addItemBool( "HasAnim1", m_hasAnim[ 0 ], false, "hasNormalAnim" );
	addItemBool( "HasAnim2", m_hasAnim[ 1 ], false, "hasNewAnim" );
	addItemBool( "HasBgColor1", m_hasBgColor[ 0 ], false, "hasnormalbgcolour" );
	addItemBool( "HasBgColor2", m_hasBgColor[ 1 ], false, "hasnewbgcolour" );
	addItemBool( "HasFgColor1", m_hasFgColor[ 0 ], true, "hasnormalfgcolour" );
	addItemBool( "HasFgColor2", m_hasFgColor[ 1 ], true, "hasnewfgcolour" );
	addItemBool( "HasFont1", m_hasFont[ 0 ], false, "hasnormalfont" );
	addItemBool( "HasFont2", m_hasFont[ 1 ], false, "hasnewfont" );
	addItemBool( "HasIcon1", m_hasIcon[ 0 ], false, "hasnormalicon" );
	addItemBool( "HasIcon2", m_hasIcon[ 1 ], false, "hasnewicon" );
	addItemString( "Anim1", *m_anim[ 0 ], false, "normalanim" );
	addItemString( "Anim2", *m_anim[ 1 ], false, "hasNewAnim" );
	addItemColor( "BgColor1", *m_bgColor[ 0 ], QColor(), "normalbgcolour" );
	addItemColor( "BgColor2", *m_bgColor[ 1 ], QColor(), "newbgcolour" );
	addItemColor( "FgColor1", *m_fgColor[ 0 ], Qt::black, "normalfgcolour" );
	addItemColor( "FgColor2", *m_fgColor[ 1 ], Qt::black, "newfgcolour" );
	addItemFont( "Font1", *m_font[ 0 ], QFont(), "normalfont" );
	addItemFont( "Font2", *m_font[ 1 ], QFont(), "newfont" );
	addItemString( "Icon1", *m_icon[ 0 ], QString(), "normalicon" );
	addItemString( "Icon2", *m_icon[ 1 ], QString(), "newicon" );

	addItemBool( "ClickCmdLeftRecheck", m_clickCommand[Left][Recheck], false, "leftrecheck" );
	addItemBool( "ClickCmdLeftReset", m_clickCommand[Left][Reset], false, "leftreset" );
	addItemBool( "ClickCmdLeftView", m_clickCommand[Left][View], false, "leftview" );
	addItemBool( "ClickCmdLeftRun", m_clickCommand[Left][Run], false, "leftrun" );
	addItemBool( "ClickCmdLeftPopup", m_clickCommand[Left][Popup], false, "leftpopup" );
	addItemBool( "ClickCmdMiddleRecheck", m_clickCommand[Middle][Recheck], false, "middlerecheck" );
	addItemBool( "ClickCmdMiddleReset", m_clickCommand[Middle][Reset], false, "middlereset" );
	addItemBool( "ClickCmdMiddleView", m_clickCommand[Middle][View], false, "middleview" );
	addItemBool( "ClickCmdMiddleRun", m_clickCommand[Middle][Run], false, "middlerun" );
	addItemBool( "ClickCmdMiddlePopup", m_clickCommand[Middle][Popup], false, "middlepopup" );
	addItemBool( "ClickCmdRightRecheck", m_clickCommand[Right][Recheck], false, "rightrecheck" );
	addItemBool( "ClickCmdRightReset", m_clickCommand[Right][Reset], false, "rigthreset" );
	addItemBool( "ClickCmdRightView", m_clickCommand[Right][View], false, "rightview" );
	addItemBool( "ClickCmdRightRun", m_clickCommand[Right][Run], false, "rightrun" );
	addItemBool( "ClickCmdRightPopup", m_clickCommand[Right][Popup], false, "rightpopup" );

	addItemBool( "Passivepopup", m_passivePopup, false, "passivepopup" );
	addItemBool( "PassiveDate", m_passiveDate, true, "passivedate" );
	addItemString( "NewCommand", *m_newcommand, QString(), "newcommand" );
	addItemString( "NewSound", *m_sound, QString(), "sound" );
}

void BoxSettings::copy( const BoxSettings& other )
{
	int xx, yy;
	
	m_boxnr = other.m_boxnr;
	*m_boxname = *other.m_boxname;
	*m_command = *other.m_command;
	
	for( xx = 0; xx < 2; ++xx )
	{
		m_hasAnim[ xx ] = other.m_hasAnim[ xx ];
		m_hasBgColor[ xx ] = other.m_hasBgColor[ xx ];
		m_hasFgColor[ xx ] = other.m_hasFgColor[ xx ];
		m_hasFont[ xx ] = other.m_hasFont[ xx ];
		m_hasIcon[ xx ] = other.m_hasIcon[ xx ];
		*m_anim[ xx ] = *other.m_anim[ xx ];
		*m_bgColor[ xx ] = *other.m_bgColor[ xx ];
		*m_fgColor[ xx ] = *other.m_fgColor[ xx ];
		*m_font[ xx ] = *other.m_font[ xx ];
		*m_icon[ xx ] = *other.m_icon[ xx ];
	}

	for( xx = 0; xx < 3; ++xx )
		for( yy = 0; yy < 5; ++yy )
			m_clickCommand[ xx ][ yy ] = other.m_clickCommand[ xx ][ yy ];

	m_passivePopup = other.m_passivePopup;
	m_passiveDate = other.m_passiveDate;
	*m_newcommand = *other.m_newcommand;
	*m_sound = *other.m_sound;

	for( xx = 0; xx < other.m_accounts->count(); ++xx )
	{
		//m_accounts->append( new AccountSettings( other.m_account->at( xx ) ) );
	}
	
	m_config = other.m_config;
}

Settings::Settings()
	: KConfigSkeleton( *m_config ),
	m_box( new QList< BoxSettings* >() )
{
	init();
}

Settings::Settings( const Settings& other )
	: KConfigSkeleton( *m_config ),
	m_box( new QList< BoxSettings* >() )
{
	init();
	copy( other );
}

Settings::~Settings()
{
	while( m_box->count() > 0 )
		delete m_box->takeLast();
	delete m_box;
}

Settings& Settings::operator=( const Settings& other )
{
	copy( other );
	return *this;
}

BoxSettings* Settings::getBox( int nr )
{
	if( nr >= m_box->count() || nr < 0 )
		return 0;
	else
		return (*m_box)[ nr ];
}

const BoxSettings* Settings::getBox( int nr ) const
{
	if( nr >= m_box->count() || nr < 0 )
		return 0;
	else
		return (*m_box)[ nr ];
}

void Settings::addBox()
{
	m_box->append( new BoxSettings( m_config, m_box->count() ) );
}

void Settings::deleteBox( int nr )
{
	while( nr + 1 < m_box->count() )
	{
		swapBox( nr, nr + 1 );
		++nr;
	}
	m_box->removeLast();
}

void Settings::swapBox( int elem1, int elem2 )
{
	if( elem1 >= m_box->count() || elem2 >= m_box->count() || elem1 < 0 || elem2 < 0 || elem1 == elem2 )
		return;
	(*m_box)[ elem1 ]->newGroup( elem2 );
	(*m_box)[ elem2 ]->newGroup( elem1 );
	m_box->swap( elem1, elem2 );
}


void Settings::usrReadConfig()
{
	int boxnr = 0;
	BoxSettings *setting;
	
	while( config()->hasGroup( QString( "korn-%1" ).arg( boxnr ) ) )
		++boxnr;

	while( boxnr > m_box->count() )
	{
		setting = new BoxSettings( m_config, m_box->count() );
		m_box->append( setting );
		setting->readConfig();
	}
	while( boxnr < m_box->count() )
		m_box->removeLast();
}

void Settings::usrWriteConfig()
{
	int count = m_box->count();
	int xx;

	for( xx = 0; xx < count; ++xx )
		(*m_box)[ xx ]->writeConfig();

	xx = count;

	while( config()->hasGroup( QString( "korn-%1" ).arg( xx ) ) )
		config()->deleteGroup( QString( "korn-%1" ).arg( xx++ ) );
}

Settings* Settings::self()
{
	if( !m_self )
	{
		m_config = new KSharedConfig::Ptr( KGlobal::config() );
		m_self = new Settings();
		m_self->readConfig();
	}

	return m_self;
}

void Settings::init()
{
	setCurrentGroup( "korn" );
	QList< KConfigSkeleton::ItemEnum::Choice > choises;
	KConfigSkeleton::ItemEnum::Choice choise;
	
	choise.name = "Horizontal";
	choise.label = "Horizontal";
	choise.whatsThis = "Horizontal";
	choises.append( choise );
	choise.name = "Vertical";
	choise.label = "Vertical";
	choise.whatsThis = "Vertical";
	choises.append( choise );
	choise.name = "Docked";
	choise.label = "Docked";
	choise.whatsThis = "Docked";
	choises.append( choise );
	KConfigSkeleton::ItemEnum *type = new KConfigSkeleton::ItemEnum( (const QString&)"korn", (const QString&)"layout", m_layout, choises, Horizontal );
	
	addItem( type, "Layout" );
	addItemBool( "UseWallet", m_useWallet, true, "usewallet" );
}

void Settings::copy( const Settings& other )
{
	int xx;
	
	m_layout = other.m_layout;
	m_useWallet = other.m_useWallet;

	while( !m_box->isEmpty() )
		delete m_box->takeFirst();

	for( xx = 0; xx < other.m_box->count(); ++xx )
		m_box->append( new BoxSettings( *other.m_box->at( xx ) ) );
}

#include "settings.moc"
