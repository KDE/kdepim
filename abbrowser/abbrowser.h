#ifndef PAB_H 
#define PAB_H 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <kapp.h>
#include <ktmainwindow.h>
 
class ContactEntry;
class ContactEntryList;
class PabWidget;

/**
 * This class serves as the main window for Pab.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Don Sanders <dsanders@kde.org>
 * @version 0.1
 */
class Pab : public KTMainWindow
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 */
	Pab();

	/**
	 * Default Destructor
	 */
	virtual ~Pab();
	ContactEntry *ce;

public slots:
	/**
	 * This is called whenever the user Drag n' Drops something into our
	 * window
	 */
        void newContact();
        void updateContact( QString addr, QString name );
	void saveCe();
	void save();
	void readConfig();
	void saveConfig();
	void undo();
	void redo();
	void updateEditMenu();
 
protected:
	/**
	 * This function is called when it is time for the app to save its
	 * properties for session management purposes.
	 */
	void saveProperties(KConfig *);

	/**
	 * This function is called when this app is restored.  The KConfig
	 * object points to the session management config file that was saved
	 * with @ref saveProperties
	 */
	void readProperties(KConfig *);
	QPopupMenu* edit;
	int undoId;
	int redoId;

private:
	PabWidget *view;
	ContactEntryList *document;
};

#endif // PAB_H 
