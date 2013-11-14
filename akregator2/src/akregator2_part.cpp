/*
    This file is part of Akregator2.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "akregator2config.h"
#include "akregator2_part.h"
#include "aboutdata.h"
#include "actionmanagerimpl.h"
#include "feediconmanager.h"
#include "framemanager.h"
#include "mainwidget.h"
#include "plugin.h"
#include "pluginmanager.h"
#include "trayicon.h"
#include "kernel.h"

#include <libkdepim/misc/broadcaststatus.h>
#include "kdepim-version.h"

#include <KCmdLineArgs>
#include <knotifyconfigwidget.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <KSaveFile>
#include <kservice.h>
#include <kxmlguifactory.h>
#include <kio/netaccess.h>
#include <KParts/GenericFactory>
#include <KParts/Plugin>
#include <KCMultiDialog>
#include <kstandardaction.h>

#include <KRss/FeedPropertiesCollectionAttribute>
#include <Akonadi/AttributeFactory>

#include <QFile>
#include <QObject>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QWidget>
#include <QDomDocument>
#include "partadaptor.h"

#include <memory>

using namespace boost;

namespace Akregator2 {

static const KAboutData &createAboutData()
{
  static Akregator2::AboutData about;
  return about;
}

K_PLUGIN_FACTORY(Akregator2Factory, registerPlugin<Part>();)
K_EXPORT_PLUGIN(Akregator2Factory(createAboutData()))

BrowserExtension::BrowserExtension( Part *p )
  : KParts::BrowserExtension( p )
  , m_part( p )
{
}

void BrowserExtension::saveSettings()
{
    m_part->saveSettings();
}

Part::Part( QWidget *parentWidget, QObject *parent, const QVariantList& )
    : inherited(parent)
    , m_shuttingDown(false)
    , m_extension( new BrowserExtension( this ) )
    , m_parentWidget(parentWidget)
    , m_actionManager( new ActionManagerImpl( this ) )
    , m_mainWidget()
    , m_dialog()

{
    Akonadi::AttributeFactory::registerAttribute<KRss::FeedPropertiesCollectionAttribute>();

    initFonts();

    setPluginLoadingMode( LoadPluginsIfEnabled );
    setPluginInterfaceVersion( AKREGATOR2_PLUGIN_INTERFACE_VERSION );

    setComponentData( Akregator2Factory::componentData() );
    setXMLFile("akregator2_part.rc", true);

    new PartAdaptor( this );
    QDBusConnection::sessionBus().registerObject("/Akregator2", this);

    FeedIconManager::self(); // FIXME: registering the icon manager dbus iface here,
                               // because otherwise we get a deadlock later

    ActionManager::setInstance( m_actionManager );

    m_mainWidget = new MainWidget( this, m_parentWidget, m_actionManager );

    connect(Kernel::self()->frameManager(), SIGNAL(signalCaptionChanged(QString)), this, SIGNAL(setWindowCaption(QString)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalStatusText(QString)), this, SLOT(slotSetStatusText(QString)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalLoadingProgress(int)), m_extension, SIGNAL(loadingProgress(int)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalCanceled(QString)), this, SIGNAL(canceled(QString)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalStarted()), this, SLOT(slotStarted()));
    connect(Kernel::self()->frameManager(), SIGNAL(signalCompleted()), this, SIGNAL(completed()));


    // notify the part that this is our internal widget
    setWidget(m_mainWidget);

    if ( Settings::showTrayIcon() && !TrayIcon::getInstance() )
    {
        TrayIcon* trayIcon = new TrayIcon( m_mainWidget->window() );
        TrayIcon::setInstance(trayIcon);
        m_actionManager->setTrayIcon(trayIcon);

        if ( isTrayIconEnabled() )
            trayIcon->setStatus( KStatusNotifierItem::Active );

        connect( m_mainWidget, SIGNAL(signalUnreadCountChanged(int)), trayIcon, SLOT(slotSetUnread(int)) );
        connect( m_mainWidget, SIGNAL(signalItemsSelected(Akonadi::Item::List)),
                this, SIGNAL(signalArticlesSelected(Akonadi::Item::List)) );
    }

    connect(kapp, SIGNAL(aboutToQuit()), this, SLOT(slotOnShutdown()));

    loadPlugins( QLatin1String("extension") ); // FIXME: also unload them!
}

void Part::loadPlugins( const QString& type )
{
    const KService::List offers = PluginManager::query( QString::fromLatin1("[X-KDE-akregator-plugintype] == '%1'").arg( type ) );

    Q_FOREACH ( const KService::Ptr& i, offers ) {
        Akregator2::Plugin* plugin = PluginManager::createFromService( i, this );
        if ( !plugin )
            continue;
        plugin->initialize();
        plugin->insertGuiClients( this );
    }
}

void Part::slotStarted()
{
    emit started(0L);
}

void Part::slotOnShutdown()
{
    m_shuttingDown = true;
    saveSettings();
    if ( m_mainWidget )
        m_mainWidget->slotOnShutdown();
    //delete m_mainWidget;
    delete TrayIcon::getInstance();
    TrayIcon::setInstance(0L);
    //delete m_actionManager;
}

void Part::addFeed() {
}

void Part::slotSettingsChanged()
{
    QStringList fonts;
    fonts.append(Settings::standardFont());
    fonts.append(Settings::fixedFont());
    fonts.append(Settings::sansSerifFont());
    fonts.append(Settings::serifFont());
    fonts.append(Settings::standardFont());
    fonts.append(Settings::standardFont());
    fonts.append("0");
    Settings::setFonts(fonts);
    if ( Settings::showTrayIcon() && !TrayIcon::getInstance() )
    {
        TrayIcon* trayIcon = new TrayIcon( m_mainWidget->window() );
        TrayIcon::setInstance(trayIcon);
        m_actionManager->setTrayIcon(trayIcon);

        if ( isTrayIconEnabled() )
            trayIcon->setStatus( KStatusNotifierItem::Active );

        connect( m_mainWidget, SIGNAL(signalUnreadCountChanged(int)), trayIcon, SLOT(slotSetUnread(int)) );
        connect( m_mainWidget, SIGNAL(signalItemsSelected(Akonadi::Item::List)),
                this, SIGNAL(signalArticlesSelected(Akonadi::Item::List)) );

        //PORTING m_mainWidget->slotSetTotalUnread();
    }
    if ( !Settings::showTrayIcon() )
    {
        TrayIcon::getInstance()->disconnect();
        delete TrayIcon::getInstance();
        TrayIcon::setInstance(0);
        m_actionManager->setTrayIcon(0);
    }

    if (Settings::minimumFontSize() > Settings::mediumFontSize())
        Settings::setMediumFontSize(Settings::minimumFontSize());
    saveSettings();
    emit signalSettingsChanged();
}

void Part::slotSetStatusText( const QString& statusText )
{
  KPIM::BroadcastStatus::instance()->setStatusMsg( statusText );
}

void Part::saveSettings()
{
    if ( m_mainWidget )
        m_mainWidget->saveSettings();
}

Part::~Part()
{
    if (!m_shuttingDown)
        slotOnShutdown();
    delete m_dialog;
}

void Part::readProperties(const KConfigGroup & config)
{
    if(m_mainWidget)
        m_mainWidget->readProperties(config);
}

void Part::saveProperties(KConfigGroup & config)
{
    if (m_mainWidget)
        m_mainWidget->saveProperties(config);
}

bool Part::openUrl(const KUrl& url)
{
    setLocalFilePath(url.toLocalFile());
    return openFile();
}

void Part::fetchAllFeeds()
{
    m_mainWidget->slotFetchAllFeeds();
}


bool Part::openFile() {
    return true;
}

bool Part::isTrayIconEnabled() const
{
    return Settings::showTrayIcon();
}

void Part::showNotificationOptions()
{
    const Akregator2::AboutData about;
    KNotifyConfigWidget::configure(m_mainWidget, about.appName() );
}

void Part::showOptions()
{
    saveSettings();

    if ( !m_dialog ) {
        m_dialog = new KCMultiDialog( m_mainWidget );
        connect( m_dialog, SIGNAL(configCommitted()),
                 this, SLOT(slotSettingsChanged()) );
        connect( m_dialog, SIGNAL(configCommitted()),
                 TrayIcon::getInstance(), SLOT(settingsChanged()) );

        // query for akregator's kcm modules
        const QString constraint = "[X-KDE-ParentApp] == 'akregator2'";
        const KService::List offers = KServiceTypeTrader::self()->query( "KCModule", constraint );
        foreach( const KService::Ptr &service, offers ) {
            m_dialog->addModule( service->storageId() );
        }
    }

    m_dialog->show();
    m_dialog->raise();
}

KParts::Part* Part::hitTest(QWidget *widget, const QPoint &globalPos)
{
/*    bool child = false;
    QWidget *me = this->widget();
    while (widget)
    {
        if (widget == me)
        {
            child = true;
            break;
        }
        if (!widget)
        {
            break;
        }
        widget = widget->parentWidget();
    }
    if (m_mainWidget && m_mainWidget->currentFrame() && child)
    {
        return m_mainWidget->currentFrame()->part();
    }
    else
    {*/
        return inherited::hitTest(widget, globalPos);
/*    }*/
}

void Part::initFonts()
{
    QStringList fonts = Settings::fonts();
    if (fonts.isEmpty())
    {
        fonts.append(KGlobalSettings::generalFont().family());
        fonts.append(KGlobalSettings::fixedFont().family());
        fonts.append(KGlobalSettings::generalFont().family());
        fonts.append(KGlobalSettings::generalFont().family());
        fonts.append("0");
    }
    Settings::setFonts(fonts);
    if (Settings::standardFont().isEmpty())
        Settings::setStandardFont(fonts[0]);
    if (Settings::fixedFont().isEmpty())
        Settings::setFixedFont(fonts[1]);
    if (Settings::sansSerifFont().isEmpty())
        Settings::setSansSerifFont(fonts[2]);
    if (Settings::serifFont().isEmpty())
        Settings::setSerifFont(fonts[3]);

    KConfigGroup conf( Settings::self()->config(), "HTML Settings");

    KConfig _konq( "konquerorrc", KConfig::NoGlobals  );
    KConfigGroup konq(&_konq, "HTML Settings");

    if (!conf.hasKey("MinimumFontSize"))
    {
        int minfs;
        if (konq.hasKey("MinimumFontSize"))
            minfs = konq.readEntry("MinimumFontSize", 8);
        else
            minfs = std::max( KGlobalSettings::generalFont().pointSize() - 2, 4 );
        Settings::setMinimumFontSize(minfs);
    }

    if (!conf.hasKey("MediumFontSize"))
    {
        int medfs;
        if (konq.hasKey("MediumFontSize"))
            medfs = konq.readEntry("MediumFontSize", 12);
        else
            medfs = KGlobalSettings::generalFont().pointSize();
        Settings::setMediumFontSize(medfs);
    }

    if (!conf.hasKey("UnderlineLinks"))
    {
        bool underline = true;
        if (konq.hasKey("UnderlineLinks"))
            underline = konq.readEntry("UnderlineLinks", false);
        Settings::setUnderlineLinks(underline);
    }

}

bool Part::handleCommandLine() {
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString addFeedGroup = !args->getOption("group").isEmpty() ?
         args->getOption("group")
         : i18n("Imported Folder");

    QStringList feedsToAdd = args->getOptionList("addfeed");

    if (feedsToAdd.isEmpty() && args->count() > 0) {
        const QString url = args->url(0).url();
        if(!url.isEmpty())
            feedsToAdd.append(url);
    }

    if (!feedsToAdd.isEmpty())
        addFeedsToGroup( feedsToAdd, addFeedGroup );
    return true;
}

void Part::addFeedsToGroup(const QStringList& urls, const QString& group)
{
    //TODO
}


} // namespace Akregator2

