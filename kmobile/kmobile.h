/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#ifndef _KMOBILE_H_
#define _KMOBILE_H_

#include <kapplication.h>
#include <kmainwindow.h>
#include <ktrader.h>

#include "kmobileview.h"

class KToggleAction;
class SystemTray;

/**
 * This class serves as the main window for KMobile.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 */
class KMobile : public KMainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    KMobile();

    /**
     * Default Destructor
     */
    virtual ~KMobile();

    KMobileView * mainView() const { return m_view; };

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

public slots:

    void saveAll();
    void restoreAll();

private slots:
    void dockApplication();
    void slotQuit();
    void showMinimized();

    void fileSave();
    void optionsShowToolbar();
    void optionsShowStatusbar();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();

    void addDevice();
    void removeDevice();
    void configDevice();

    void renameDevice();

    void optionsPreferences();

    void newToolbarConfig();

    void changeStatusbar(const QString& text);

private:
    void setupAccel();
    void setupActions();

    bool queryExit();
    bool queryClose();

private:
    KConfig *m_config;

    KMobileView *m_view;
    SystemTray *m_systemTray;

    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
};

#endif // _KMOBILE_H_
