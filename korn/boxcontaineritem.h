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

#ifndef MK_BOXCONTAINERITEM_H
#define MK_BOXCONTAINERITEM_H

/**
 * @file
 * 
 * This class contains a class BoxContainerItem
 */

#include "accountmanager.h"
#include <QPixmap>

class KornMailSubject;
class Settings;

class KActionCollection;
class KConfig;
class KMenu;
class K3Process;

class QColor;
class QLabel;
class QPixmap;
class QString;
template< class T > class QList;

/**
 * This class provide a base for an item. This item should be
 * shown in a BoxContainer, and represent one number somewhere.
 * This also is the DCOP-interface for such a box.
 *
 * @author Mart Kelder <mart.kde@hccnet.nl>
 */
class BoxContainerItem : public AccountManager
{ Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.korn.BoxContainerItem")
public:
	/**
	 * Standaard QObject-constuctor
	 * This constructor implements the default arguments for any QObject.
	 * Note that is does not give a name to DCOPObject; that name
	 * is set in the readConfig-function.
	 *
	 * @param parent The parent of this object, See Object::QObject
	 * @see BoxContainerItem::readConfig
	 */
	BoxContainerItem( QObject * parent = 0 );
	
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
	 *
	 * @param config The KConfigGroup-object which contains the configuration of this box.
	 * @param index The index of the box used in the config-file
	 */
	virtual void readConfig( BoxSettings *settings, BoxSettings *, const int index );

public slots:
	/**
	 * This functions sets the number to be displayed. It should be reimplemented.
	 * @param count The number of new messages waiting
	 * @param newMessages Are there any new messages added?
	 */
	virtual void setCount( const int count, const bool newMessages ) = 0;
	
	/**
	 * This functions execute a given command
	 *
	 * @param cmd The command to be executed.
	 */
	virtual void runCommand( const QString& cmd );

	/**
	 * This function displays the (kde) help dialog
	 */
	void help();
	/**
	 * This function calls a bug report dialog
	 */
	void reportBug();
	/**
	 * This function calls the about dialog
	 */
	void about();
	
	/**
	 * This functions should be called if a mouse-button has been pressed.
	 * This handles the connected events of it.
	 * 
	 * @param button The button that was pressed, See Qt::MouseButton
	 */
	void mouseButtonPressed( Qt::MouseButton button );
protected:
	/**
	 * This function filles a KMenu-reference. The target is
	 * to set in all implementations the same KMenu-content.
	 * Because some implementations (DockedItem) got a KMenu
	 * by itself, this only changes a KMenu instance.
	 * @param menu The menu to be changed.
	 * @param actions The actions to which the items should be added.
	 */
	void fillKMenu( KMenu* menu, KActionCollection* actions );

	/**
	 * This displays the passive popup.
	 *
	 * @param parent The Winget of the visual widget
	 * @param list List with the first (five) subjects
	 * @param total The total numbers of unread mail
	 * @param accountName The name of the account it belongs to
	 * @param date Should the date be displayed?
	 */
	void showPassivePopup( QWidget* parent, QList< KornMailSubject >* list, int total, const QString& accountName, bool date );
		
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
	
	/**
	 * This function draws a specified configuration into a label
	 * @param the resulting pixmap (size should be ok already)
	 * @param label The label to be filled
	 * @param count the number of new messages
	 * @param newMessages are the messages new?
	 * @return true if the pixmap is empty; false otherwise
	 */
	bool makePixmap( QPixmap& result, const int count, const bool newMessages );
	
private:
	/**
	 * This function helps to make a pixmap
	 * @param icon The icon to be set in
	 * @param fgColour the colour of the foreground
 	 * @param font the font; 0 is default font.
	 * @param count the number of messages
	 */
	static QPixmap calcComplexPixmap( const QPixmap &icon, const QColor& fgColour, const QFont& font, const int count );
	
	/**
	 * This functions sets a movie to a specified label
	 * @param label The label to set the movie in
	 * @param anim The path to the animation
	 */
	void setAnimIcon( QLabel* label, const QString& anim );

protected:
	BoxSettings *_settings;
	
private slots:
	/**
	 * This slot calls recheck()
	 */
	void slotRecheck() { recheck(); }
	/**
	 * This slot calls reset()
	 */
	void slotReset() { reset(); }
	/**
	 * This slot calls view()
	 */
	void slotView() { view(); }
	/**
	 * This slot calls runCommand()
	 */
	void slotRunCommand() { runCommand(); }
	/**
	 * This slot calls popup()
	 */
	void slotPopup() { popup(); }
	/**
	 * This slot calls showConfig()
	 */
	void slotConfigure() { showConfig(); }
public Q_SLOTS:
	/**
	 * This call immediately checked all accounts of this box.
	 */
	Q_SCRIPTABLE void recheck();
	/**
	 * This call reset the number of unread messages to 0.
	 */
	Q_SCRIPTABLE void reset();
	/**
	 * This call popup's a window with the headers of the new messages.
	 */
	Q_SCRIPTABLE void view();
	/**
	 * This program executes the command as setup'ed.
	 */
	Q_SCRIPTABLE void runCommand(); //Possible_unsafe?
	/**
	 * This function lets the popup-menu's be displayed.
	 */
	Q_SCRIPTABLE void popup();
	
	/**
	 * This function lets the user edit the configuration
	 */
	Q_SCRIPTABLE void showConfig();

	/**
	 * With this DCOP-call, a user can start the account.
	 */
	Q_SCRIPTABLE void startTimer();
	/**
	 * With this DCOP-call, a user can stop the account.
	 */
	Q_SCRIPTABLE void stopTimer();
signals:
	/**
	 * This signal is emitted when the user whants to configure something.
	 */
	void showConfiguration();
	

private slots:
	/**
	 * This slot is called when a K3Process-instance needs to be deleted:)
	 * @param proc The instance of the instance which must be deleted.
	 */
	void processExited( K3Process* proc );
};

#endif //MK_BOXCONTAINERITEM_H
