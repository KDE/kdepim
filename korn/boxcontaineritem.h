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

#ifndef MK_BOXCONTAINERITEM_H
#define MK_BOXCONTAINERITEM_H

#include "accountmanager.h"
#include <dcopobject.h>

class KornMailSubject;

class KActionCollection;
class KConfig;
class KPopupMenu;
class KProcess;

class QColor;
class QLabel;
//template< class T > class QPtrList;
class QString;

/**
 * This class provide a base for an item. This item should be
 * shown in a BoxContainer, and represent one number somewhere.
 * This also is the DCOP-interface for such a box.
 * @author Mart Kelder <mart.kde@hccnet.nl>
 */
class BoxContainerItem : public AccountManager, public DCOPObject
{ Q_OBJECT
  K_DCOP
public:
	/**
	 * Standaard QObject-constuctor
	 * This constructor implements the default arguments for any QObject.
	 * Note that is does not give a name to DCOPObject; that name
	 * is set in the readConfig-function.
	 * @param parent The parent of this object, @see Object::QObject
	 * @param name The name of this object, @see QObject::QObject
	 * @see BoxContainerItem::readConfig
	 */
	BoxContainerItem( QObject * parent = 0, const char * name = 0 );
	
	/**
	 * The default destructor. This only removes _command-pointer.
	 */
	~BoxContainerItem();
	
	/**
	 * If this function should call a "show"-function of its widget.
	 */
	virtual void showBox() = 0;
	
	/**
	 * This function reads the config. It stores the readed values in the class.
	 * It also sets the DCOPObject-name.
	 * @param config The KConfigGroup-object which contains the configuration of this box.
	 * @param index The index of the box used in the config-file
	 */
	virtual void readConfig( KConfig* config, const int index );

public slots:
	/**
	 * This functions sets the number to be displayed. It should be reimplemented.
	 * @param count The number of new messages waiting
	 * @param newMessages Are there any new messages added?
	 */
	virtual void setCount( const int count, const bool newMessages ) = 0;
	
	virtual void runCommand( const QString& cmd );

	void help();
	void reportBug();
	void about();
	
	/**
	 * This functions should be called if a mouse-button has been pressed.
	 * This handles the connected events of it.
	 * 
	 * @param button The button that was pressed, @see Qt::ButtonState
	 */
	void mouseButtonPressed( Qt::ButtonState button );
protected:
	/**
	 * This function filles a KPopupMenu-reference. The target is
	 * to set in all implementations the same KPopupMenu-content.
	 * Because some implementations (DockedItem) got a KPopupMenu
	 * by itself, this only changes a KPopupMenu instance.
	 * @param menu The menu to be changed.
	 * @param actions The actions to which the items should be added.
	 */
	void fillKPopupMenu( KPopupMenu* menu, KActionCollection* actions ) const;

	/**
	 * This displays the passive popup.
	 *
	 * @param parent The Winget of the visual widget
	 * @param list List with the first (five) subjects
	 * @param total The total numbers of unread mail
	 * @param accountName The name of the account it belongs to
	 * @param date Should the date be displayed?
	 */
	void showPassivePopup( QWidget* parent, QPtrList< KornMailSubject >* list, int total, const QString& accountName, bool date );
		
	//this functions should be reimplemented
	/**
	 * This function is called when the implementation of the class
	 * should display the popup-menu.
	 */
	virtual void doPopup() = 0;
	
	/**
	 * This function draws a specified configuration into a label
	 * @param label The label to be filled
	 * @param count the number of new messages
	 * @param newMessages are the messages new?
	 */
	void drawLabel( QLabel *label, const int count, const bool newMessages );
	
private:
	/**
	 * This function helps to make a pixmap
	 * @param icon The icon to be set in
	 * @param fgColour the colour of the foreground
 	 * @param font the font; 0 is default font.
	 * @param count the number of messages
	 */
	static QPixmap calcComplexPixmap( const QPixmap &icon, const QColor& fgColour, const QFont* font, const int count );
	
	/**
	 * This functions sets a movie to a specified label
	 * @param label The label to set the movie in
	 * @param anim The path to the animation
	 */
	void setAnimIcon( QLabel* label, const QString& anim );
	
private slots:
	void slotRecheck() { recheck(); }
	void slotReset() { reset(); }
	void slotView() { view(); }
	void slotRunCommand() { runCommand(); }
	void slotPopup() { popup(); }
	void slotConfigure() { showConfig(); }
public:
k_dcop:
	/**
	 * This call immediately checked all accounts of this box.
	 */
	void recheck();
	/**
	 * This call reset the number of unread messages to 0.
	 */
	void reset();
	/**
	 * This call popup's a window with the headers of the new messages.
	 */
	void view();
	/**
	 * This program executes the command as setup'ed.
	 */
	void runCommand(); //Possible_unsafe?
	/**
	 * This function lets the popup-menu's be displayed.
	 */
	void popup();
	
	/**
	 * This function lets the user edit the configuration
	 */
	void showConfig();

	/**
	 * With these DCOP-calls, a user can start and stop the accounts.
	 */
	void startTimer();
	void stopTimer();
signals:
	/**
	 * This signal is emitted when the user whants to configure something.
	 */
	void showConfiguration();
	

private slots:
	/**
	 * This slot is called when a KProcess-instance needs to be deleted:)
	 * @param proc The instance of the instance which must be deleted.
	 */
	void processExited( KProcess* proc );

protected:
	//This settings are stored here because every implementation needs them.
	QString *_icons[ 2 ];
	QString *_anims[ 2 ];
	QColor *_fgColour[ 2 ];
	QColor *_bgColour[ 2 ];
	QFont *_fonts[ 2 ];
	
private:
	QString *_command;
	bool _recheckSettings[ 3 ];
	bool _resetSettings[ 3 ];
	bool _viewSettings[ 3 ];
	bool _runSettings[ 3 ];
	bool _popupSettings[ 3 ];
};

#endif //MK_BOXCONTAINERITEM_H
