/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "homepage.h"

#include <khtml_part.h>
#include <qfile.h>
//Added by qt3to4:
#include <Q3CString>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <khtmlview.h>
#include <aboutdata.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <qapplication.h>
#include <qstringlist.h>
#include <kplugininfo.h>

#include "engine.h"
#include "enginedata.h"
#include "devicesconfig.h"

#include "kmobiletools_cfg.h"
#include "devicemenu.h"
#include "engineslist.h"
#include "smslist.h"
#include "contactslist.h"
// FIXME port to D-Bus!
#if 0
#include "mainIFace_stub.h"
#include "deviceIFace_stub.h"
#endif

#include <iostream>
using namespace KMobileTools;

class homepagePartPrivate {
public:
    homepagePartPrivate() : i_infopage(-1), p_engine(0)
    {}
    QString content;
    int i_infopage;
    KMobileTools::Engine * p_engine;
};

homepagePart::homepagePart(QWidget *parent, const char *name)
 : KHTMLPart(parent, parent)
{
    d=new homepagePartPrivate;
    setObjectName(QLatin1String(name));
    view()->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding/*, 5, 2*/) );
    connect( browserExtension(), SIGNAL( openUrlRequest(const KUrl &, const KParts::URLArgs &) ),
             this, SLOT( openUrlRequest(const KUrl &) ) );
    connect( this, SIGNAL(popupMenu(const QString &, const QPoint &) ),
             this, SLOT(slotContextMenu(const QString&, const QPoint& ) ) );
//     openURL("http://www.kde.org");
//     setupHP();
    setStatusMessagesEnabled(true);
}


homepagePart::~homepagePart()
{
    end();
    delete d;
}


#include "homepage.moc"

/*!
    \fn homepagePart::refreshData()
 */
void homepagePart::refreshData(const QString &data)
{
    QString location = KStandardDirs::locate( "data", "kmobiletools/about/main.html" );
    QFile fcontent(location);
    fcontent.open(QIODevice::ReadOnly);
    d->content = QString(fcontent.readAll() );
    fcontent.close();
    d->content = d->content.arg( KStandardDirs::locate( "data", "kmobiletools/about/kde_infopage.css" ) );
    d->content = d->content.arg( KStandardDirs::locate( "data", "kmobiletools/about/kmobiletools.css" ) );
//     if ( kapp->reverseLayout() )
//         content = content.arg( "@import \"%1\";" ).arg( KStandardDirs::locate( "data", "kmobiletools/about/kde_infopage_rtl.css" ) );
//     else
//          d->content = d->content.arg( "" );
    begin( KUrl( location ) );

    QString appName( i18n( "KMobileTools" ) );
    QString catchPhrase( i18n( "Get Synced!" ) );
    QString quickDescription( i18n( "a KDE Mobile Phone Syncing and Management tool" ) );
    d->content=d->content.arg( QFont().pointSize() + 2 ).arg( appName )
            .arg( catchPhrase ).arg( quickDescription ).arg( data );
    write( d->content );
    end();
//     cout << content << "\n";
    debugPage( d->content );
}


/*!
    \fn homepagePart::openUrlRequest(const KUrl &url, const KParts::URLArgs &args=KParts::URLArgs())
 */
void homepagePart::openUrlRequest(const KUrl &url)
{
    kDebug() << "openUrlRequest: " << url << endl;
    if(url.protocol() == "mobile")
        emit switchDevice( url.path() );
    if(url.protocol() == "config")
        emit configCmd( url.path() );
    if(url.protocol() == "infopage")
        emit infopage ( url.path().toInt() );
    if(url.protocol().contains("device") )
        emit deviceCMD( url );
}

void homepagePart::printIndexPage()
{
    QString htmlData;
    htmlData="<h4 style='text-align:center; margin-top: 0px;'>%1 %2</h4>\n\
            <table align=\"center\">\n";
    htmlData=htmlData.arg( i18n("Welcome to KMobileTools")).arg(AboutData().version() );
    QStringList devices=KMobileTools::MainConfig::devicelist();
    if( ! (devices.count()) ) /// Assuming that the first part is always the homepage
    {
        htmlData+="<tr><td align=\"left\">%1</td></tr>\n";
        htmlData+="<tr><td align=\"left\"><a href=\"config:newDevWiz\">\n\t<img src=\"%2\" align=\"absmiddle\"> <b>%3</b></a></td></tr>\
                <tr><td align=\"left\"><a href=\"config:configDevices\">\n\t<img src=\"%4\" align=\"absmiddle\"> <b>%5</b></a></td></tr>";
        htmlData=htmlData.arg( i18n("KMobileTools handles your mobile phone devices, can read, write and send sms, synchronize your addressbook, and much more") )
                .arg( KIconLoader::global()->iconPath("plasmagik" /* was "wizard"*/, -K3Icon::SizeHuge) )
                .arg( i18n("Add a new mobile phone device") )
                .arg( KIconLoader::global()->iconPath("package-utilities" /* was blockdevice"*/, -K3Icon::SizeHuge) )
                .arg( i18n("Configure devices") );
    } else
    for(QStringList::Iterator it = devices.begin(); it != devices.end(); ++it )
    {
        htmlData+="<a href=\"mobile:%3\"><img src=\"%1\" align=\"left\" style=\"background-image : url('%5');\"><p><font size=\"+1\">&nbsp;&nbsp;%2</font></p><p>&nbsp;&nbsp;%4</p><br clear=\"ALL\"></a><br>";
        QString iconDev;
        KPluginInfo *infos=KMobileTools::EnginesList::instance()->engineInfo( DEVCFG(*it)->engine() );
        if(infos) iconDev=infos->icon();
        kDebug() << "Loaded icon: " << iconDev << endl;

//         MainIFace_stub *mainStub=new MainIFace_stub(kapp->dcopClient(), "kmobiletools", "KMobileTools" );
//         bool isLoaded=mainStub->deviceIsLoaded( *it );
        bool isLoaded=KMobileTools::EnginesList::instance()->find(*it);

//         if( !isLoaded ) iconDev.append("_d");
        QString iconBG;
        if ( !isLoaded ) iconBG = "closedphone"; else iconBG="kmobiletools";
        QString stringStatus;
        if( isLoaded )
        {
#if 0
            deviceIFace=new DeviceIFace_stub( kapp->dcopClient(), "kmobiletools", Q3CString((*it).latin1() ) );
            if ( deviceIFace->isConnected() )
#endif
            if ( KMobileTools::EnginesList::instance()->find(*it)->constEngineData().phoneConnected() ) // ### FIXME
                stringStatus=i18n("Device connected");
            else
            {
                stringStatus=i18n("Device disconnected");
                iconDev="overlaydisc";
            }
//            delete deviceIFace;
        }
        else stringStatus=i18n("Not loaded");
        htmlData=htmlData.arg( KIconLoader::global()->iconPath(iconDev, -K3Icon::SizeHuge) )
            .arg( DEVCFG(*it)->devicename() )
            .arg( QString::fromLatin1( KUrl::toPercentEncoding( *it, "/" ) ) )
            .arg( stringStatus )
            .arg( KIconLoader::global()->iconPath(iconBG, -K3Icon::SizeHuge) );

        htmlData+="</p>\n";
//        delete mainStub;
    }
    htmlData+="</table>";
    refreshData(htmlData);
}

void homepagePart::printInfoPage(int i, KMobileTools::Engine *engine)
{
    if(!engine) return;
    printInfoPage(i, DEVCFG(engine->objectName())->devicename(), engine);
}

void homepagePart::printInfoPage(int i, const QString &mobileName, KMobileTools::Engine *engine)
{
    d->p_engine=engine;
    d->i_infopage=i;
    QString htmlData;
    QString devname;
    if(engine) devname=engine->objectName(); else devname=KMobileTools::DevicesConfig::deviceGroup(mobileName); // @TODO a bit tricky, try doing it better
    htmlData="<h2 style='text-align:center; margin-top: 0px;'><img src=\"%1\" align=\"absmiddle\"> %2</h2>";
    htmlData=htmlData.arg(KMobileTools::DevicesConfig::deviceTypeIconPath( devname,-K3Icon::SizeHuge) )
                     .arg( mobileName );

    switch( i ){
    case 1:
        htmlData+="<ul><li><b>%1</b></li></ul>";
        htmlData=htmlData.arg(i18n("Phone details") );
        htmlData+="<p><b>%1</b> %2</p><p><b>%3</b> %4</p><p><b>%5</b> %6</p><p><b>%7</b> %8</p><div align='right'><a href=\"infopage:0\">%9</a></div>";
        htmlData=htmlData.arg(i18n("Manufacturer: ") ).arg(engine->constEngineData().manufacturer() )
            .arg(i18n("Model: ") ).arg(engine->constEngineData().model() )
            .arg(i18n("IMEI: ") ).arg(engine->constEngineData().imei() )
            .arg(i18n("Revision: ") ).arg(engine->constEngineData().revision() )
            .arg(i18n("Phone overview") );
        break;
    case 2:
        htmlData+="<center><b>%1</b></center><br><br>%3<br><br><br><center><img src=\"%2\"></center><br>";
        htmlData=htmlData
                .arg(i18n("Searching Mobile Phone") )
                .arg(KGlobal::dirs ()->findResource("data", "kmobiletools/progress.gif") )
                .arg(i18n("Please wait while KMobileTools tries to find the right device for your mobile phone.") );
        break;
    default:
        if( engine->constEngineData().phoneConnected() )
        {
            SMSList *l=engine->constEngineData().smsList();
            htmlData+="<ul><li><b>%8</b></li></ul><p>%1 %2</p><p>%3 %4</p><div align='right'><a href=\"infopage:1\">%7</a></div>";
            htmlData=htmlData
                    .arg( QString("<a href=\"%1:sms\">").arg( devname ) +
                    htmlIcon("mail_get",-K3Icon::SizeSmallMedium) )
                    .arg(i18nc("sms count in device homepage", "%1 received SMS (%2 unread).",
                        ( l->count( SMS::Unread | SMS::Read, SMS::Phone | SMS::SIM) ),
                        ( l->count( SMS::Unread, SMS::Phone | SMS::SIM) ) ) + "</a>")
//                     .arg( htmlIcon("phonecall",-K3Icon::SizeSmallMedium) ).arg("No new calls")
                    .arg(  QString("<a href=\"%1:phonebook\">").arg(devname ) + htmlIcon("personal",-K3Icon::SizeSmallMedium) )
                    .arg(i18np("%1 contact stored in phonebook.", "%1 contacts stored in phonebook.", engine->constEngineData().contactsList()->count())
                    ) + "</a>";
        } else
        {
            htmlData+="<ul><li><b>%8</b></li></ul><p>%1 %2</p><p>%3 %4</p><div align='right'><a href=\"infopage:1\">%7</a></div>";
            htmlData=htmlData
                    .arg(htmlIcon("stop",-K3Icon::SizeSmallMedium) )
                    .arg(QString("<a href=\"%1:tryconnect\">%2</a>").arg(devname).arg(i18n("Device disconnected. Click here to retry connect.") ) )
                    .arg(htmlIcon("configure",-K3Icon::SizeSmallMedium) )
                    .arg(QString("<a href=\"%1:configure\">%2</a>").arg(devname).arg(i18n("Click here to configure this mobile phone.") ) );
        }
        htmlData=htmlData.arg(i18n("Phone details") )
                .arg(i18n("Phone overview") );
        break;
    }
    refreshData( htmlData );
}

void homepagePart::debugPage(const QString &htmlData)
{
    kDebug() << "debugPage::" << htmlData.left(10) << "...\n";
    QTextStream str(stdout);
    str << htmlData;
}

const QString homepagePart::htmlIcon(const QString &iconName, int groupOrSize)
{
    return KIconLoader::global()->iconPath(iconName, groupOrSize, false)
    .prepend("<img align=\"absmiddle\" src=\"").append("\">");
}


/*!
    \fn homepagePart::slotContextMenu(const QString& urlString, const QPoint& point )
 */
void homepagePart::slotContextMenu(const QString& urlString, const QPoint& point )
{
    kDebug() << "homepagePart::slotContextMenu(\"" << urlString << "\", " << point << ") engine:" << d->p_engine <<";\n";
    KUrl url(urlString);
//     if(urlString==QString::null && p_engine) url=KUrl::fromPathOrUrl( QString("mobile:%1").arg(p_engine->objectName() ));
    KMenu *m_popup=0;
    if(url.protocol() == "mobile" || d->p_engine)
    {
        bool eng_loaded;
        QString eng_name;
        if(!d->p_engine)
        {
            eng_loaded=(KMobileTools::EnginesList::instance()->namesList(false).contains(url.path() )>0);
            eng_name=url.path();
        } else
        {
            eng_loaded=true;
            eng_name=d->p_engine->objectName();
        }
        m_popup=new deviceMenu( eng_loaded, d->p_engine, 0, eng_name );
        connect(m_popup, SIGNAL(switchDevice(const QString &) ), SIGNAL(switchDevice(const QString &) ) );
        connect(m_popup, SIGNAL(loadDevice(const QString &) ), SIGNAL(loadDevice(const QString &) ) );
        connect(m_popup, SIGNAL(unloadDevice(const QString &) ), SIGNAL(unloadDevice(const QString &) ) );
        connect(m_popup, SIGNAL(configure(const QString &) ), SIGNAL(configCmd(const QString &)) );
        connect(m_popup, SIGNAL(sendURL(const KUrl&) ), this, SLOT(openUrlRequest(const KUrl&) ) );
    }
    if( ! m_popup ) return;
    m_popup->exec(point);
}

void homepagePart::guiActivateEvent( KParts::GUIActivateEvent *event )
{
    KHTMLPart::guiActivateEvent(event);
    emit setStatusBarText( QString("KMobileTools") );
}

void homepagePart::partActivateEvent( KParts::PartActivateEvent *event )
{
    QApplication::sendEvent(this, (new KParts::GUIActivateEvent( event->activated() ) )  );
}

int homepagePart::currentInfoPage() { return d->i_infopage; }
