#ifndef KADDRESSBOOKMAIN_H
#define KADDRESSBOOKMAIN_H

#include <qptrlist.h>
#include <kapplication.h>
#include <kmainwindow.h>
#include <kaction.h>

#include "kaddressbookiface.h"
#include "kaddressbook.h"

class ActionManager;

/**
 * This class serves as the main window for KAddressBook.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Don Sanders <dsanders@kde.org>
 * @version 0.1
 */
class KAddressBookMain : public KMainWindow, virtual public KAddressBookIface
{
    Q_OBJECT
  public:
    KAddressBookMain();
    virtual ~KAddressBookMain();

  public slots:
    virtual void addEmail( QString addr ) { mWidget->addEmail( addr ); }

    virtual ASYNC showContactEditor( QString uid ) { mWidget->showContactEditor( uid ); }
    virtual void newContact() { mWidget->newContact(); }
    virtual QString getNameByPhone( QString phone ) { return mWidget->getNameByPhone( phone ); }
    virtual void save() { mWidget->save(); }
    virtual void exit() { close(); }
    

  protected:
    void initActions();
    
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

    virtual bool queryClose();
 
  protected slots:
    void configureToolbars();
    void configureKeys();

    void slotNewToolbarConfig();
    
  private:
    KAddressBook *mWidget;
    ActionManager *mActionManager;
};

#endif
