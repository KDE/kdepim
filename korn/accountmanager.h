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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MK_ACCOUNTMANAGER_H
#define MK_ACCOUNTMANAGER_H

#include <qobject.h>

class DCOPDrop;
class KornSubjectsDlg;
class KKioDrop;
class KMailDrop;

class KConfig;

template< class T > class QPtrList;
template< class T, class W > class QMap;


/**
 * This class manage the different accounts.
 *
 * This class reads the config and makes the accounts,
 * and it communicate with the boxes.
 */
class AccountManager : public QObject
{ Q_OBJECT
public:
	/**
	 * Constructor, parameters are directed to QObject.
	 */
	AccountManager( QObject * parent = 0, const char * name = 0 );

	/**
	 * Destructor
	 */
	~AccountManager();
	
	/**
	 * This function is used to read the config.
	 *
	 * @param config The KConfig instance to the configuration
	 * @param index The index of the box. As there are different boxes,
	 * every box have it's own index. This number is used to get the
	 * right config-information out of the config parameter.
	 */
	virtual void readConfig( KConfig* config, const int index );

	/**
	 * This write the configuration into a file. Things that must be right is
	 * for example the reset number. The configuration isn't saved through this
	 * method; configurations are saved in the configurations classes.
	 *
	 * @param config The KConfig instance to which the configuration is written.
	 * @param index The index of the box.
	 */
	virtual void writeConfig( KConfig* config, const int index );
	
	/**
	 * This method makes a QString which can be used for a tooltip.
	 * In it, all accounts are summed and the number of new messages of
	 * every account is added.
	 *
	 * @return A string that can be used for the Tooltip of the box.
	 */
	QString getTooltip() const;
protected:
	/**
	 * This function is called when the number of emails has changed.
	 * Boxes must override this method and update the information.
	 *
	 * @param numberOfNewMessages The number of unread messages.
	 * @param newMessages Are there any new messages (important for displaying it)?
	 */
	virtual void setCount( int numberOfNewMessages, bool newMessages ) = 0;

	/**
	 * This functions sets a new Tooltip. Boxes must override this method
	 * @param tooltip The tooltip to be set.
	 */
	virtual void setTooltip( const QString& tooltip ) = 0;

	/**
	 * This funtion is called if @p command have to be executed.
	 * For example, if new email has arrived, and the user setuped KOrn
	 * to execute a command. Boxes must override this function.
	 */
	virtual void runCommand( const QString& command ) = 0;
	
	/**
	 * This function can be called by classes that inherit this class.
	 * If this function is called, all account which are part of this
	 * box are rechecked.
	 */
	void doRecheck();
	
	/**
	 * If this method is called, the number of new messages of all of its account
	 * is resetted.
	 */
	void doReset();

	/**
	 * If this function is called, a windows with shows the message headers will popup.
	 */
	void doView();

	/**
	 * These functions are called if the user wants to start or stop the account being triggered.
	 */
	void doStartTimer();
	void doStopTimer();
	
private:
	struct Dropinfo
	{
		int index;
		int msgnr;
		bool newMessages;
		int reset;
	};
	
	QPtrList< KKioDrop > *_kioList;
	QPtrList< DCOPDrop > *_dcopList;
	
	QMap< KMailDrop*, Dropinfo* > *_dropInfo;
	
	static KornSubjectsDlg *_subjectsDlg;
	
private:
	int totalMessages();
	bool hasNewMessages();
	void playSound( const QString& );
private slots:
	void slotChanged( int, KMailDrop* );
	void slotValidChanged( bool );
};

#endif //MK_ACCOUNTMANAGER_H

