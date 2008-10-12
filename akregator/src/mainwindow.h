/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#ifndef AKREGATOR_MAINWINDOW_H
#define AKREGATOR_MAINWINDOW_H

#include <kapplication.h>
#include <kparts/mainwindow.h>
#include <kparts/browserinterface.h>
#include <kparts/browserextension.h>
#include <kio/job.h>

#include <QPointer>

class KSqueezedTextLabel;
class KParts::BrowserExtension;

namespace KPIM
{
    class StatusbarProgressWidget;
}

namespace Akregator
{

class Part;
class MainWindow;

class BrowserInterface : public KParts::BrowserInterface
{
    Q_OBJECT

public:
    BrowserInterface(Akregator::MainWindow *shell, const char *name );

private:
    Akregator::MainWindow *m_shell;
};


/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 */
class MainWindow : public KParts::MainWindow
{
    Q_OBJECT
public:
    explicit MainWindow( QWidget* parent=0, Qt::WindowFlags f=KDE_DEFAULT_WINDOWFLAGS );
    ~MainWindow();

    /**
     * Creates the progress widget in the status bar and the ProgressDialog
     * and connects them.
     */
    void setupProgressWidgets();


    /**
     Loads the part
     @return Whether the part has been successfully created or not.
     */
     bool loadPart();

public slots:
    void slotClearStatusText();
    void slotSetStatusBarText(const QString &c);
    void slotToggleVisibility();

protected:
    /**
     * This method is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfigGroup &);

    /**
     * This method is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(const KConfigGroup &);

    /**
     * This is called when exiting akregator in order to be able to restore
     * its state next time it starts.
     */
    void autoSaveProperties();

    /**
     * This function is called when starting akregator and it will restore
     * the state akregator the last time it was closed if possible.
     */
    void autoReadProperties();
    /**
     *  Reimplemented to save settings
     */
    bool queryExit();

    /**
     * Reimplemented to say if app will be running in system tray if necessary
     */
    bool queryClose();


protected slots:
    void slotQuit();

private:

    KParts::BrowserExtension *browserExtension(KParts::ReadOnlyPart *p);

private slots:

    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void applyNewToolbarConfig();

private:
    BrowserInterface *m_browserIface;

    QPointer<Akregator::Part> m_part;
    KPIM::StatusbarProgressWidget *m_progressBar;
    KSqueezedTextLabel *m_statusLabel;
};

} // namespace Akregator

#endif // AKREGATOR_MAINWINDOW_H
