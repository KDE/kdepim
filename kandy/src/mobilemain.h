#ifndef MOBILEMAIN_H
#define MOBILEMAIN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kmainwindow.h>

class CommandScheduler;

/**
 * This class serves as the main window for MobileMain.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Cornelius Schumacher <schumacher@kde.org>
 * @version 0.1
 */
class MobileMain : public KMainWindow
{
    Q_OBJECT
  public:
    /**
     * Default Constructor
     */
    MobileMain(CommandScheduler *);

    /**
     * Default Destructor
     */
    virtual ~MobileMain();

  public slots:
    void setConnected(bool);

  signals:
    void showTerminalWin();
    void showPreferencesWin();

    void modemConnect();
    void modemDisconnect();

  protected:
    /**
     * Overridden virtuals for Qt drag 'n drop (XDND)
     */
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

    bool queryClose();

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


  private slots:
    void showTerminal();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void optionsPreferences();
    void newToolbarConfig();

    void showStatusMessage(const QString& text);
    void showTransientStatusMessage(const QString& text);
    void changeCaption(const QString& text);


  private:
    void setupActions();

  private:
    MobileGui *mView;
};

#endif // MOBILEMAIN_H
