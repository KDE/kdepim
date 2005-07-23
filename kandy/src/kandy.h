/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KANDY_H
#define KANDY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <kapplication.h>
#include <kmainwindow.h>
 
#include "kandyview.h"

class QPrinter;

class KandyPrefsDialog;

/**
 * This class serves as the main window for Kandy.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Cornelius Schumacher <schumacher@kde.org>
 * @version 0.1
 */
class Kandy : public KMainWindow
{
    Q_OBJECT
  public:
    /**
     * Default Constructor
     */
    Kandy(CommandScheduler *);

    /**
     * Default Destructor
     */
    virtual ~Kandy();

    /**
     * Use this method to load whatever file/URL you have
     */
    void load(const QString& url);
    void save(const QString& url);

  public slots:
    void setTitle();

    void modemConnect();

    void showErrorMessage( const QString & );

  signals:
    void showMobileWin();
    void connectStateChanged(bool);

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
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void filePrint();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void optionsPreferences();
    void modemDisconnect();
    void showMobileGui();
    void newToolbarConfig();

    void changeStatusbar(const QString& text);
    void changeCaption(const QString& text);


  private:
    void setupAccel();
    void setupActions();

  private:
    CommandScheduler *mScheduler;
  
    KandyView *mView;

    QPrinter   *mPrinter;

    KAction *mConnectAction;
    KAction *mDisconnectAction;
    
    QString mFilename;

    KandyPrefsDialog *mPreferencesDialog;
};

#endif // KANDY_H
