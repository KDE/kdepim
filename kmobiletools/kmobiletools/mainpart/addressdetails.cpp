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
 
#include "addressdetails.h"

#include <kabc/phonenumber.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <QTextDocument>

#include <libkmobiletools/kmobiletoolshelper.h>
#include <libkmobiletools/engine.h>
#include <libkmobiletools/devicesconfig.h>
#include <libkmobiletools/popupnumber.h>
#include <libkmobiletools/popupaddressee.h>

using namespace KMobileTools;

addressDetails::addressDetails(QWidget *parentWidget, const QString &objectname,QObject *parent)
 : KHTMLPart(parentWidget, parent)
{
    setObjectName(objectname);
    kDebug() << "addressDetails:: device name: " << objectName() << endl;
    connect(this, SIGNAL(popupMenu(const QString &,const QPoint &)), SLOT(popupMenu ( const QString &, const QPoint &) ) );
    connect( browserExtension(), SIGNAL( openUrlRequest(const KUrl &, const KParts::URLArgs &) ), this, SLOT( openUrlRequest(const KUrl &) ) );
    showHP();
}


addressDetails::~addressDetails()
{
}


#include "addressdetails.moc"

const QString addressDetails::getTemplate()
{
    return  KMobiletoolsHelper::getTemplate()
            .arg( KMobiletoolsHelper::getFooterLink( i18nc("Add new contact", "Add new"), "wizard", "contact:add" ) )
            .arg( KMobiletoolsHelper::getFooterLink( i18nc("Fetch phonebook from the mobile", "Reload"), "reload", "contact:refresh" ) )
            .arg( KMobiletoolsHelper::getFooterLink( i18nc("Import phonebook", "Import"), "revert", "contact:import" ) )
            .arg( KMobiletoolsHelper::getFooterLink( i18nc("Export phonebook", "Export"), "fileexport", "contact:export" ) );
}
/*!
    \fn addressDetails::showAddressee(KABC::Addressee &addressee)
 */
void addressDetails::showAddressee(const KABC::Addressee &addressee, bool readOnly)
{
    if(addressee.isEmpty()) return;
    ro=readOnly;
    QString htmlData; // ="<img src=\"%1\" align=\"middle\"> <b><i>%2</b>";
//     htmlData=htmlData.arg( KIconLoader::global()->iconPath("personal", KIcon::Desktop, false) ).arg( addressee.formattedName() );
    /*="<div style=\"border-bottom-style : hidden; border-bottom-width : 10; border-left-style : hidden;\
        border-left-width : 5; border-right-style : hidden; border-right-width : 5; border-spacing : 5; border-top-style :\
        hidden; border-top-width : 10;background-color: %1; color: %2\">\
    <img src=\"%3\" align=\"middle\"> <b><i>%4</b></i></div>\n\n";*/
    QString renderData=getTemplate().arg( KIconLoader::global()->iconPath("personal", -K3Icon::SizeHuge, false) )
            .arg( addressee.formattedName() );
    if(readOnly) renderData=renderData.arg(QString() ).arg(QString() );
    else renderData=renderData
            .arg( KMobiletoolsHelper::getFooterLink( i18nc("Edit Contact", "Edit") , "edit", "contact:edit" ) )
            .arg( KMobiletoolsHelper::getFooterLink( i18nc("Delete Contact", "Delete") , "editdelete", "contact:del" ) );
    /*
    htmlData = htmlData
                .arg( KGlobalSettings::activeTitleColor().name() )
                .arg( KGlobalSettings::activeTextColor().name() )
                .arg( KIconLoader::global()->iconPath("personal", K3Icon::Desktop, false) ).arg( addressee.formattedName() );
    */
    KABC::PhoneNumber::List numberList = addressee.phoneNumbers();
    for ( KABC::PhoneNumber::List::Iterator it = numberList.begin(); it != numberList.end(); it++ )
    {
        htmlData += QString("<p><b>%1:</b> <a href=\"number:").arg( (*it).typeLabel() );
        htmlData+=QString::fromLatin1( KUrl::toPercentEncoding( (*it).number(), "/" ) )
                + "\">" +Qt::escape( (*it).number() ) + "</a></p>\n";
    }

    QString storedOn;
    switch( addressee.custom("KMobileTools","memslot").toInt() ){
        case KMobileTools::Engine::PB_SIM:
            storedOn=PB_SIM_TEXT;
            break;
        case KMobileTools::Engine::PB_Phone:
            storedOn=PB_PHONE_TEXT;
            break;
        case KMobileTools::Engine::PB_DataCard:
            storedOn=PB_DATACARD_TEXT;
            break;
        default:
            storedOn=i18n("not available");
    }
    
    htmlData+=i18n("<p>Stored on: <b>%1</b></p>", storedOn);
/*    htmlData+=QString("<div style=\"border-bottom-style : hidden; border-bottom-width : 10; border-left-style : hidden;\
            border-left-width : 5; border-right-style : hidden; border-right-width : 5; border-spacing : 5; border-top-style :\
            hidden; border-top-width : 10; position: absolute; bottom: 10px; left: 10px; right: 10px; background-color: %3; color: %4\" align=\"right\"><a href=\"contact:edit\">%1</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"contact:refresh\">%2</a></div>")
            .arg( i18n("Edit contact") ).arg(i18n ("Reload list") )
            .arg( KGlobalSettings::activeTitleColor().name() )
            .arg( KGlobalSettings::activeTextColor().name() );*/
    renderData=renderData.arg(htmlData );
    begin();
    write( renderData );
    end();
    p_addressee=addressee;
}


/*!
    \fn addressDetails::popupMenu ( const QString &url, const QPoint &point)
 */
void addressDetails::popupMenu ( const QString &url, const QPoint &point)
{
    KUrl kurl(url);
    KMenu *popup=0;
    if(kurl.protocol()=="number")
        popup=new popupNumber(objectName(), kurl.path(), 0);
    if( url.isNull() )
    {
        if(p_addressee.isEmpty()) return;
        popup=new popupAddressee(objectName(), p_addressee, 0, ro );
        connect(popup, SIGNAL( editClicked(KABC::Addressee) ), this, SIGNAL(editClicked(KABC::Addressee) ) );
        connect(popup, SIGNAL( delContact() ), this, SIGNAL(delContact() ) );
    }
    if(!popup) return;
    popup->exec(point);
}


void addressDetails::openUrlRequest(const KUrl &url)
{
    if(url.path() == "refresh") emit refreshClicked();
    if(url.path() == "edit") emit editClicked(p_addressee);
    if(url.path() == "add" ) emit addContact();
    if(url.path() == "del" ) emit delContact();
    if(url.path() == "import") emit importPB();
    if(url.path() == "export") emit exportPB();
    if(url.protocol() == "number") emit dial(url.path() );
}


/*!
    \fn addressDetails::showHP()
 */
void addressDetails::showHP()
{
    begin();
    write( getTemplate().arg(KIconLoader::global()->iconPath("kontact_contacts", -K3Icon::SizeHuge, false) )
            .arg( i18n("%1 AddressBook", DEVCFG(objectName() )->devicename() ) )
            .arg("")            .arg("")
            .arg( i18n("<p><i>Click a contact on the left list to see details, or \"Reload\" to update the contacts list.</i></p>") )
         );
    end();
    p_addressee=KABC::Addressee();
}
