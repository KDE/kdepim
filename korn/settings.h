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


#ifndef MK_SETTINGS_H
#define MK_SETTINGS_H

#include <kconfigskeleton.h>

class BoxSettings;
class QString;

template< class T, class S > class QMap;

class AccountSettings : public KConfigSkeleton
{ Q_OBJECT
public:
	AccountSettings( KSharedConfig::Ptr *config, BoxSettings *parent, int boxnr, int accountnr );
	AccountSettings( const AccountSettings& other );
	~AccountSettings();

	AccountSettings& operator=( const AccountSettings &other );

	void newGroup( int boxnr, int accountnr );

	QString protocol() const;
	void setProtocol( const QString& val );

	QString accountName() const;
	void setAccountName( const QString& val );

	int accountNumber() const { return m_accountnr; }

	bool useBoxSettings() const { return m_useBoxSettings; }
	void setUseBoxSettings( bool val ) { m_useBoxSettings = val; }

	int reset() const { return m_reset; }
	void setReset( int val ) { m_reset = val; }

	int interval() const { return m_interval; }
	void setInterval( int val ) { m_interval = val; }

	bool passivePopup() const;
	void setPassivePopup( bool val ) { m_passivePopup = val; }

	bool passiveDate() const;
	void setPassiveDate( bool val ) { m_passiveDate = val; }

	QString command() const;
	void setCommand( const QString& val );

	QString sound() const;
	void setSound( const QString& val );

	QMap< QString, QString > readEntries() const;
	void writeEntries( const QMap< QString, QString > &settings );
protected:
	virtual void usrReadConfig();
	virtual void usrWriteConfig();
private:
	void init();
	void copy( const AccountSettings& );

signals:
	void resetChanged();
private:
	//Parent
	BoxSettings *m_box;
	KSharedConfig::Ptr *m_config;
	int m_boxnr;
	int m_accountnr;
	//General entries
	QString *m_protocol;
	QString *m_name;
	bool m_useBoxSettings;
	int m_reset;
	int m_interval;

	//New mail settings
	bool m_passivePopup;
	bool m_passiveDate;
	QString *m_command;
	QString *m_sound;

	QMap< QString, QString > *m_settings;
};

class BoxSettings : public KConfigSkeleton
{ Q_OBJECT
public:
	BoxSettings( KSharedConfig::Ptr *config, int boxnr );
	BoxSettings( const BoxSettings& other );
	~BoxSettings();

	enum State { Normal = 0, New = 1 };
	enum Action { Recheck = 0, Reset = 1, View = 2, Run = 3, Popup = 4 };
	
	void newGroup( int boxnr );

	QString boxName() const;
	void setBoxName( const QString& val );

	QString command() const;
	void setCommand( const QString& val );

	bool hasAnimation( State state ) const { return m_hasAnim[ state ]; }
	void setHasAnimation( State state, bool val ) { m_hasAnim[ state ] = val; }

	bool hasBackgroundColor( State state ) const { return m_hasBgColor[ state ]; }
	void setHasBackgroundColor( State state, bool val ) { m_hasBgColor[ state ] = val; }

	bool hasForegroundColor( State state ) const { return m_hasFgColor[ state ]; }
	void setHasForegroundColor( State state, bool val ) { m_hasFgColor[ state ] = val; }
	
	bool hasFont( State state ) const { return m_hasFont[ state ]; }
	void setHasFont( State state, bool val ) { m_hasFont[ state ] = val; }

	bool hasIcon( State state ) const { return m_hasIcon[ state ]; }
	void setHasIcon( State state, bool val ) { m_hasIcon[ state ] = val; }

	QString animation( State state ) const;
	void setAnimation( State state, const QString& val );

	QColor backgroundColor( State state ) const;
	void setBackgroundColor( State state, const QColor& val );

	QColor foregroundColor( State state ) const;
	void setForegroundColor( State state, const QColor& val );
	
	QFont font( State state ) const;
	void setFont( State state, const QFont& val );

	QString icon( State state ) const;
	void setIcon( State state, const QString& val );

	bool clickCommand( Qt::MouseButton button, Action action );
	void setClickCommand( Qt::MouseButton, Action action, bool val );

	bool passivePopup() const { return m_passivePopup; }
	void setPassivePopup( bool val ) { m_passivePopup = val; }

	bool passiveDate() const { return m_passiveDate; }
	void setPassiveDate( bool val ) { m_passiveDate = val; }

	QString newCommand() const;
	void setNewCommand( const QString& val );

	QString sound() const;
	void setSound( const QString& val );

	AccountSettings* account( int accountnr );
	void addAccount();
	void deleteAccount( int accountnr );
	void swapAccounts( int account1, int account2 );
protected:
	virtual void usrReadConfig();
	virtual void usrWriteConfig();
private:
	void init();
	void copy( const BoxSettings& other );
private:
	enum MouseButton { Left = 0, Middle = 1, Right = 2 };
	
	int m_boxnr;

	//General settings
	QString *m_boxname;
	QString *m_command;
	
	//Visual settings
	bool m_hasAnim[2];
	bool m_hasBgColor[2];
	bool m_hasFgColor[2];
	bool m_hasFont[2];
	bool m_hasIcon[2];
	QString *m_anim[2];
	QColor *m_bgColor[2];
	QColor *m_fgColor[2];
	QFont *m_font[2];
	QString *m_icon[2];

	//Click commands
	bool m_clickCommand[3][5];
	
	//New mail settings
	bool m_passivePopup;
	bool m_passiveDate;
	QString *m_newcommand;
	QString *m_sound;
	
	//Accounts
	QList< AccountSettings* > *m_accounts;
	
	KSharedConfig::Ptr *m_config;
};

class Settings : public KConfigSkeleton
{ Q_OBJECT
public:
	Settings();
	Settings( const Settings& );
	~Settings();

	Settings& operator=( const Settings& other );

	enum Layout { Horizontal, Vertical, Docked };

	void setLayout( Layout val ) { m_layout = (int)val; }
	Layout layout() const { return (Settings::Layout)m_layout; }

	void setUseWallet( bool val ) { m_useWallet = val; }
	bool useWallet() const { return m_useWallet; }

	BoxSettings* getBox( int nr );
	const BoxSettings* getBox( int nr ) const;
	void addBox();
	void deleteBox( int nr );
	void swapBox( int elem1, int elem2 );

	static Settings* self();
protected:
	virtual void usrReadConfig();
        virtual void usrWriteConfig();
private:
	void init();
	void copy( const Settings& other );
private:
	int m_layout;
	bool m_useWallet;
	QList< BoxSettings* > *m_box;

	static KSharedConfig::Ptr *m_config;
	static Settings* m_self;
};

#endif //MK_SETTINGS_H

