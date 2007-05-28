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
#include "kmobiletoolshelper.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include "engine.h"
#include "engineslist.h"
#include <stdlib.h>
#include <kabc/stdaddressbook.h>
#include <qfile.h>

#include "kmobiletools_cfg.h"
#include "devicesconfig.h"
#include "contactslist.h"
#include "enginedata.h"


using namespace KMobileTools;

class KMobiletoolsHelperPrivate {
    public:
        KMobiletoolsHelperPrivate()
    : p_sysTray(0)
        {}
        KSystemTrayIcon *p_sysTray;
};

K_GLOBAL_STATIC(KMobileTools::KMobiletoolsHelper, p_instance)


void KMobileTools::Thread::usleep(ulong us)
{
    QThread::usleep(us);
}

void KMobileTools::Thread::msleep(ulong ms)
{
    QThread::msleep(ms);
}

void KMobileTools::Thread::sleep(ulong s)
{
    QThread::sleep(s);
}


KMobiletoolsHelper::KMobiletoolsHelper(QObject *parent)
 : QObject(parent), d(new KMobiletoolsHelperPrivate)
{
}


KMobiletoolsHelper::~KMobiletoolsHelper()
{
    delete d;
}

KSystemTrayIcon *KMobiletoolsHelper::systray()
{
    return d->p_sysTray;
}

void KMobiletoolsHelper::setSystray(KSystemTrayIcon *s)
{
    d->p_sysTray=s;
}

KMobiletoolsHelper *KMobiletoolsHelper::instance()
{
    return p_instance;
}



bool KMobiletoolsHelper::compareNumbers(const QString &number1, const QString &number2)
{
//     kDebug() << "KMobiletoolsHelper::compareNumbers(" << number1 << ", " << number2 << ")\n";
    if( abs( number1.length() - number2.length() ) > 4 ) return false;
    int minlen=(number1.length() < number2.length() ) ? number1.length() -1 : number2.length() -1 ;
//     kDebug() << "Number1.length()=" << number1.length() << "; Number2.length()=" << number2.length() << "; minlen=" << minlen << endl;
//     kDebug() << "Now comparing " << number1.right( minlen ) << "==" << number2.right( minlen ) << endl;
    return number1.right( minlen )==number2.right( minlen );
}


QString KMobiletoolsHelper::removeIntPrefix( const QString &number )
{
    if ( number.startsWith( "00" ) )
        return QString("0")+number.mid( 4 );
    if ( number.startsWith( "+" ) )
        return QString("0")+number.mid( 3 );
    return number;
}

QString KMobiletoolsHelper::translateNumber( const QString &s_number )
{
    if(! s_number.length() ) return QString();
//     kDebug() << "KMobiletoolsHelper::translateNumber(" << s_number << ")\n";
    KMobileTools::Engine *engine;
    QString retval;
    QList<KMobileTools::Engine*>::ConstIterator it=KMobileTools::EnginesList::instance()->begin(), itEnd=KMobileTools::EnginesList::instance()->end();
    for( ; it!=itEnd; ++it)
    {
        engine = *it;
        retval=translateNumber(s_number, engine->engineData()->contactsList() );
        if(  retval != s_number ) return retval;
    }
    retval=translateNumber(s_number, new ContactsList( KABC::StdAddressBook::self()->allAddressees () ) );
    if(retval!=s_number) return retval;
    return s_number;
}

QString KMobiletoolsHelper::translateNumber( const QString &s_number, ContactsList *phoneBook )
{
    if(! s_number.length() ) return QString();
//     kDebug() << "KMobiletoolsHelper::translateNumber(" << s_number << ", phonebook)\n";
    ContactsListIterator it_addressee(*phoneBook);
    KABC::Addressee addressee;
    KABC::PhoneNumber::List numberslist;
    KABC::PhoneNumber::List::ConstIterator it_phonenumber;
    while( it_addressee.hasNext() )
    {
        addressee=it_addressee.next();
//         ++it_addressee;
        numberslist=addressee.phoneNumbers();
        for( it_phonenumber=numberslist.begin(); it_phonenumber!= numberslist.end(); ++it_phonenumber )
            if( compareNumbers(s_number, (*it_phonenumber).number() ) ) return addressee.formattedName() ;
    }
    return s_number;
}

#include "kmobiletoolshelper.moc"
QString KMobiletoolsHelper::getTemplate()
{
    QString head_css="<HTML><HEAD><script>\n\
            function resize() {\n\
            var headerHeight, winHeight, footerHeight, calcHeight, contDiv;\n\
            headerHeight=document.getElementById('header').offsetHeight;\n\
            footerHeight=document.getElementById('footer').offsetHeight;\n\
            winHeight=window.innerHeight;\n\
            contDiv=document.getElementById('content');\n\
            calcHeight=winHeight-footerHeight-headerHeight-21;\n\
            calcHeight=calcHeight+ \"px\"\n\
            contDiv.style.height=calcHeight;\n\
            // alert (\"Calculated height: \" + calcHeight + \"; Current: \" + contDiv.style.height + \"\\nWindow height: \" + winHeight +\"\\nHeader height: \" + headerHeight + \"\\nFooter height: \" + footerHeight);\n\
            }</script>\n\
            <style>a:link { text-decoration: none; color: %1;} \n\
            a:hover{ text-decoration: underline; color: %2;} \n\
            a:active{text-decoration: underline; color: %3 } \n\
            a:visited{ text-decoration: none; color: %4 }</style>\n\
            </head><body onResize=\"resize();\" onLoad=\"resize();\">\n";
    QString body_css=head_css;
    head_css=head_css.arg( KGlobalSettings::activeTextColor().name() )
            .arg( KGlobalSettings::highlightColor().name() )
            .arg( KGlobalSettings::highlightColor().name() )
            .arg( KGlobalSettings::visitedLinkColor().name() );
    body_css=body_css.arg( KGlobalSettings::linkColor().name() )
            .arg( KGlobalSettings::visitedLinkColor().name() )
            .arg( KGlobalSettings::visitedLinkColor().name() )
            .arg( KGlobalSettings::visitedLinkColor().name() );
    QString s_template=body_css+
            "<div id=\"header\" style=\"border-bottom-style : hidden; border-bottom-width : 10; border-left-style : hidden;\n\
            border-left-width : 5; border-right-style : hidden; border-right-width : 5; border-spacing : 5; border-top-style :\n\
            hidden; border-top-width : 10;background-color: %1; color: %2; min-height : 64px; \">\n\
            <img src=\"%5\" align=\"middle\" style=\"float : left; padding-left : 1px;\"> <b><i>%6</b></i></div><div id=\"content\" style=\"height: 20px; color: %3; overflow : auto;\">%9</div>\n\n";
    s_template=s_template
            .arg( KGlobalSettings::activeTitleColor().name() )
            .arg( KGlobalSettings::activeTextColor().name() )
            .arg( KGlobalSettings::textColor().name() );
    s_template+= QString("<div id=\"footer\" style=\"border-bottom-style : hidden; border-bottom-width : 10; border-left-style : hidden;\n\
            border-left-width : 5; border-right-style : hidden; border-right-width : 5; border-spacing : 5; border-top-style :\n\
            hidden; border-top-width : 10; position: absolute; bottom: 10px; left: 10px; right: 10px; background-color: %1; color: %2\" align=\"right\">\n")
            .arg( KGlobalSettings::activeTitleColor().name() )
            .arg( KGlobalSettings::activeTextColor().name() );
    s_template+= QString("<table><tr><td></td><td></td><td>%7</td><td>%8</td></tr>\
            <tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr></table></body></html>");
    return s_template;
}

QString  KMobiletoolsHelper::getFooterLink(const QString &text, const QString &iconName, const QString &url)
{
    QString retval="<a href=\"%1\"><img src=\"%2\" align=\"absmiddle\"> <font size=-1 color=\"%3\">%4</font></a>";
    retval=retval.arg(url)
            .arg( KIconLoader::global()->iconPath( iconName, K3Icon::Toolbar, false) )
            .arg( KGlobalSettings::activeTextColor().name() )
            .arg( text );
    return retval;
}


/*!
    \fn KMobileTools::KMobiletoolsHelper::mkMailDir(const QString &dirname)
 */
void KMobileTools::KMobiletoolsHelper::createMailDir(const QString &dirname)
{
    QString dirpath=KMobileTools::DevicesConfig::prefs(dirname)->maildir_path();
    QDir().mkdir( dirpath );
    QDir mailbox(dirpath);
    dirpath=mkMailDir(dirpath, true);
    dirpath=mkMailDir(dirpath+QDir::separator()+ KMobileTools::DevicesConfig::prefs(dirname)->devicename() , true );
    QString simPath=mkMailDir(dirpath+QDir::separator()+i18nc("SIM MailDir", "SIM"), true);
    QString phonePath=mkMailDir(dirpath+QDir::separator()+i18nc("Phone MailDir", "Phone"), true);
    mkMailDir(simPath+QDir::separator()+i18nc("Incoming MailDir", "Incoming"));
    mkMailDir(simPath+QDir::separator()+i18nc("Outgoing MailDir", "Outgoing"));
    mkMailDir(phonePath+QDir::separator()+i18nc("Incoming MailDir", "Incoming"));
    mkMailDir(phonePath+QDir::separator()+i18nc("Outgoing MailDir", "Outgoing"));
}
/*!
        \fn KMobileTools::KMobiletoolsHelper::mkMailDir(const QString &dirname, bool isParent)
 */
QString KMobileTools::KMobiletoolsHelper::mkMailDir(const QString &dirname, bool isParent)
{
    kDebug() << "Trying to make " << dirname << endl;
    QDir MailDir(dirname);
    MailDir.mkdir( dirname );
    MailDir.mkdir( dirname + QDir::separator()+( "cur" ) );
    MailDir.mkdir( dirname + QDir::separator()+( "new" ) );
    MailDir.mkdir( dirname + QDir::separator()+( "tmp" ) );
    if(isParent)
    {
        QString dircontainer=MailDir.dirName().prepend( '.' ).append( ".directory" );
        MailDir.cdUp();
        MailDir.mkdir( MailDir.absolutePath().append( QDir::separator() ).append( dircontainer ) );
        MailDir.cd( MailDir.absolutePath().append( QDir::separator() ).append( dircontainer ) );
        return MailDir.path();
    }
    MailDir.cd( dirname );
    return MailDir.path();
}

QString KMobileTools::KMobiletoolsHelper::shortMonthNameEng(int month)
{
    switch( month ){
        case 2:
            return "Feb";
        case 3:
            return "Mar";
        case 4:
            return "Apr";
        case 5:
            return "May";
        case 6:
            return "Jun";
        case 7:
            return "Jul";
        case 8:
            return "Aug";
        case 9:
            return "Sep";
        case 10:
            return "Oct";
        case 11:
            return "Nov";
        case 12:
            return "Dec";
        default:
            return "Jan";
    }
}

QString KMobileTools::KMobiletoolsHelper::shortWeekDayNameEng(int day)
{
    switch( day ){
        case 2:
            return "Tue";
        case 3:
            return "Wen";
        case 4:
            return "Thu";
        case 5:
            return "Fri";
        case 6:
            return "Sat";
        case 7:
            return "Sun";
        default:
            return "Mon";
    }
}
QStringList KMobileTools::KMobiletoolsHelper::getStdDevices(int connectionFlags, const QStringList &oldDevices)
{
    QStringList newDevices=getStdDevices(connectionFlags);
    if(oldDevices.isEmpty()) return newDevices;
    QStringList oldDevicesNew=oldDevices;
    QStringList stdDevices=getStdDevices( USB | IRDA | Bluetooth | Serial );
    for(QStringList::Iterator it=stdDevices.begin(); it!=stdDevices.end(); ++it)
        oldDevicesNew.removeAll( *it );
    newDevices+=oldDevicesNew;
    return newDevices;
}

QStringList KMobileTools::KMobiletoolsHelper::getStdDevices(int connectionFlags)
{
    QStringList retval;
    if(connectionFlags & USB)
    {
        for(int i=0; i<10; i++) retval+=( QString("/dev/ttyACM%1").arg(i) );
        for(int i=0; i<10; i++) retval+=( QString("/dev/ttyUSB%1").arg(i) );
    }
    if(connectionFlags & IRDA)
    {
        for(int i=0; i<10; i++) retval+=( QString("/dev/ircomm%1").arg(i) );
    }
    if(connectionFlags & Bluetooth)
        for(int i=0; i<10; i++) retval+=( QString("/dev/rfcomm%1").arg(i) );
    if((connectionFlags & Serial) )
        for(int i=0; i<4; i++) retval+=( QString("/dev/ttyS%1").arg(i) );
    return retval;
}




/*!
    \fn KMobileTools::KMobiletoolsHelper::memorySlotsDescriptions(const char *slot, int type)
 */
QString KMobileTools::KMobiletoolsHelper::memorySlotsDescriptions(const QString &slot, int type)
{
    // Phonebook Slots
    if(slot=="DC") return i18nc("Phonebook memory slot", "Dialed Calls");
    if(slot=="EN") return i18nc("Phonebook memory slot", "Emergency Numbers");
    if(slot=="FD") return i18nc("Phonebook memory slot", "Fixed Dialing");
    if(slot=="MC") return i18nc("Phonebook memory slot", "Missed Calls");
    if(slot=="ON" || slot=="OW") return i18nc("Phonebook memory slot", "Own Numbers");
    if(slot=="RC") return i18nc("Phonebook memory slot", "Received Calls");
    if(slot=="MD" || slot=="LD" ) return i18nc("Phonebook memory slot", "Last Number Redial Memory");
    if(slot=="MV") return i18nc("Phonebook memory slot", "Voice Dialing");
    if(slot=="HP") return i18nc("Phonebook memory slot", "Hierarchical Contacts List");
    if(slot=="BC") return i18nc("Phonebook memory slot", "Own Business Card");
    // Now almost SMS ones
    if(slot=="BM") return i18nc("SMS memory slot", "SMS Stored in Volatile Memory");
    if(slot=="SR") return i18nc("SMS memory slot", "Status Report");
    if(slot=="TL") return i18nc("SMS memory slot", "SMS Templates Storage");
    if(slot=="IM") return i18nc("SMS memory slot", "Incoming SMS Storage");
    if(slot=="OM") return i18nc("SMS memory slot", "Outgoing SMS Storage");
    // Mixed/Shared slots
    if(slot=="ME") {
        if(type==PhoneBook) return i18nc("Phonebook memory slot", "Contacts Stored in Phone Memory.");
        else return i18nc("SMS memory slot", "SMS Stored in Phone Memory");
    }
    if(slot=="MT") {
        if(type==PhoneBook) return i18nc("Phonebook memory slot", "Contacts Stored in All Memory");
        else return i18nc("SMS memory slot", "SMS Stored in All Memory");
    }
    if(slot=="SM") {
        if(type==PhoneBook) return i18nc("Phonebook memory slot", "Contacts Stored in SIM Memory.");
        else return i18nc("SMS memory slot", "SMS Stored in SIM Memory");
    }
    if(slot=="TA") {
        if(type==PhoneBook) return i18nc("Phonebook memory slot", "Contacts Stored in Datacard Memory.");
        else return i18nc("SMS memory slot", "SMS Stored in Datacard Memory");
    }
    return QString() ;
}
