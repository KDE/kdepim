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

#include "mainwindow.h"
#include "akregator_part.h"
#include "akregatorconfig.h"

//settings

#include <dcopclient.h>

#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kkeydialog.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/partmanager.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kurl.h>

#include "progressdialog.h"
#include "statusbarprogresswidget.h"
#include "trayicon.h"

#include <qmetaobject.h>
#include <qpen.h>
#include <qpainter.h>
#include <private/qucomextra_p.h>
#include <qtimer.h>


namespace Akregator {

BrowserInterface::BrowserInterface( MainWindow *shell, const char *name )
    : KParts::BrowserInterface( shell, name )
{
    m_shell = shell;
}

MainWindow::MainWindow()
    : KParts::MainWindow( 0L, "akregator_mainwindow" ){
    // set the shell's ui resource file
    setXMLFile("akregator_shell.rc");

    m_browserIface=new BrowserInterface(this, "browser_interface");

    m_part=0;

    // then, setup our actions

    toolBar()->show();
    // and a status bar
    statusBar()->show();

    int statH=fontMetrics().height()+2;
    m_statusLabel = new KSqueezedTextLabel(this);
    m_statusLabel->setTextFormat(Qt::RichText);
    m_statusLabel->setSizePolicy(QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed ));
    m_statusLabel->setMinimumWidth( 0 );
    m_statusLabel->setFixedHeight( statH );
    statusBar()->addWidget (m_statusLabel, 1, false);

    setupActions();
    createGUI(0L);
}

bool MainWindow::loadPart()
{
    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self()->factory("libakregatorpart");
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        m_part = static_cast<Akregator::Part*>(factory->create(this, "akregator_part", "KParts::ReadOnlyPart" ));

        if (m_part)
        {
            // tell the KParts::MainWindow that this is indeed the main widget
            setCentralWidget(m_part->widget());

            connect(m_part, SIGNAL(setWindowCaption (const QString &)), this, SLOT(setCaption (const QString &)));

            connect(TrayIcon::getInstance(), SIGNAL(quitSelected()), this, SLOT(slotQuit()));
            // and integrate the part's GUI with the shell's
            connectActionCollection(m_part->actionCollection());
            createGUI(m_part);
            browserExtension(m_part)->setBrowserInterface(m_browserIface);
            setAutoSaveSettings();
            return true;
        }
        return false;
    }
    else
    {
        KMessageBox::error(this, i18n("Could not find the Akregator part; please check your installation."));
        return false;
    }

}

void MainWindow::setupProgressWidgets()
{
    KPIM::ProgressDialog *progressDialog = new KPIM::ProgressDialog( statusBar(), this );
    progressDialog->raise();
    progressDialog->hide();
    m_progressBar = new KPIM::StatusbarProgressWidget( progressDialog, statusBar() );
    m_progressBar->show();
    statusBar()->addWidget( m_progressBar, 0, true );
}

MainWindow::~MainWindow()
{
}

void MainWindow::setCaption(const QString &a)
{
    KParts::MainWindow::setCaption(a);
}

void MainWindow::setupActions()
{
    connectActionCollection(actionCollection());

    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    setStandardToolBarMenuEnabled(true);
    createStandardStatusBarAction();

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void MainWindow::saveProperties(KConfig* config)
{
    if (!m_part)
        loadPart();

    static_cast<Akregator::Part*>(m_part)->saveProperties(config);
    config->writeEntry("docked", isHidden());

    //delete m_part;
}

void MainWindow::readProperties(KConfig* config)
{
    if (!m_part)
        loadPart();
    static_cast<Akregator::Part*>(m_part)->readProperties(config);
    
    if (Settings::showTrayIcon() && config->readBoolEntry("docked", false)) 
        hide();
    else
        show();
}

void MainWindow::optionsConfigureKeys()
{
    KKeyDialog dlg( true, this );

    dlg.insert(actionCollection());
    if (m_part)
        dlg.insert(m_part->actionCollection());

    dlg.configure();
}

void MainWindow::optionsConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());

    // use the standard toolbar editor
    KEditToolbar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}



void MainWindow::applyNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}


KParts::BrowserExtension *MainWindow::browserExtension(KParts::ReadOnlyPart *p)
{
    return KParts::BrowserExtension::childObject( p );
}


// from konqmainwindow
void MainWindow::connectActionCollection( KActionCollection *coll )
{
    if (!coll) return;
    connect( coll, SIGNAL( actionStatusText( const QString & ) ),
              m_statusLabel, SLOT( setText( const QString & ) ) );
    connect( coll, SIGNAL( clearStatusText() ),
             this, SLOT( slotClearStatusText() ) );
}

bool MainWindow::queryExit()
{
    kdDebug() << "MainWindow::queryExit()" << endl;
    if ( !kapp->sessionSaving() )
    {
        delete m_part; // delete that here instead of dtor to ensure nested khtmlparts are deleted before singleton objects like KHTMLPageCache
        m_part = 0;
    }
    else
        kdDebug("MainWindow::queryExit(): saving session");

    return KMainWindow::queryExit();
}

void MainWindow::slotQuit()
{
    if (TrayIcon::getInstance())
        TrayIcon::getInstance()->hide();
    kapp->quit();
}

bool MainWindow::queryClose()
{
    if (kapp->sessionSaving() || TrayIcon::getInstance() == 0 || TrayIcon::getInstance()->isHidden() )
    {
        return true;
    }
    else
    {
        QPixmap shot = TrayIcon::getInstance()->takeScreenshot();

        // Associate source to image and show the dialog:
        QMimeSourceFactory::defaultFactory()->setPixmap("systray_shot", shot);
        KMessageBox::information(this, i18n( "<qt><p>Closing the main window will keep Akregator running in the system tray. Use 'Quit' from the 'File' menu to quit the application.</p><p><center><img source=\"systray_shot\"></center></p></qt>" ), i18n( "Docking in System Tray" ), "hideOnCloseInfo");
        hide();
        return false;
    }
}


void MainWindow::slotClearStatusText()
{
    m_statusLabel->setText(QString());
}

void MainWindow::slotSetStatusBarText( const QString & text )
{
    m_statusLabel->setText(text);
}

} // namespace Akregator

#include "mainwindow.moc"


// vim: set et ts=4 sts=4 sw=4:
