#ifndef PAB_H 
#define PAB_H 

#include <kapp.h>
#include <ktmainwindow.h>
 
#include <Entity.h>
#include <Field.h>
#include <KAddressBookInterface.h>

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

public slots:
	/**
	 * This is called whenever the user Drag n' Drops something into our
	 * window
	 */
	void slotDropEvent(/*KDNDDropZone **/);
  void newContact();
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
};

#endif // PAB_H 
