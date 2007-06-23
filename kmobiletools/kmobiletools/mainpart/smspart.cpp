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
#include "smspart.h"
#include <klocale.h>
#include <kiconloader.h>
#include <q3stylesheet.h>
#include <kmenu.h>
#include <khtmlview.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <stdio.h> ///@TODO remove after testing.
#include <QTextDocument>

#include <libkmobiletools/kmobiletoolshelper.h>
#include <popupnumber.h>
#include <libkmobiletools/sms.h>

#define strBegin QString("<a href=\"smsctl:refresh\">%1</a> %2").arg(i18n("Click here") ).arg(i18n("to fetch all SMS from the phone") )

using namespace KMobileTools;
smsPart::smsPart(QWidget *parentWidget, const char *widgetname, QObject *parent, const QString& name, GUIProfile prof)
    : KHTMLPart(parentWidget, parent, prof), p_sms(0)
{
    setObjectName(name);
//     begin();
//     write(strBegin);
//     end();
    writeHome();
    setEncoding( "utf16", true );
    setCharset( "utf16", true );
    connect(browserExtension(), SIGNAL(openUrlRequest(const KUrl &, const KParts::URLArgs &) ), this, SLOT(openUrlRequest(const KUrl& ) ) );
    connect(this, SIGNAL(popupMenu( const QString&, const QPoint& )), this, SLOT(slotPopupMenu( const QString&, const QPoint& )) );
//     connect(browserExtension(), SIGNAL(selectionInfo(const QString & )), this, SLOT(slotSelectedText( const QString& )) );
    setJScriptEnabled(true);
}


smsPart::~smsPart()
{
}


#include "smspart.moc"

const QString smsPart::getTemplate()
{
    return  KMobiletoolsHelper::getTemplate()
            .arg( KMobiletoolsHelper::getFooterLink( i18nc("Write a new SMS", "Write new"), "wizard", "sms:add" ) )
            .arg( KMobiletoolsHelper::getFooterLink( i18nc("Reload SMS List", "Reload"), "reload", "sms:refresh" ) )
            .arg(/* KMobiletoolsHelper::getFooterLink( i18nc("Import SMSList", "Import"), "revert", "sms:import" )*/ "&nbsp;" )
            .arg( KMobiletoolsHelper::getFooterLink( i18nc("Export SMSList", "Export"), "fileexport", "sms:export" ) );
}


/*!
    \fn smsPart::openUrlRequest(const KUrl &url)
 */
void smsPart::openUrlRequest(const KUrl &url)
{
    if( url.protocol() == "sms" )
    {
        if(url.path() == "refresh" ) emit getSMSList();
        if(url.path() == "add" ) emit writeNew();
        if(url.path() == "import") emit importList();
        if(url.path() == "export") emit exportList();
        if(url.path() == "exportToCSV") emit exportListToCSV();
        if(!p_sms) return;
        if(url.path() == "reply") slotReply();
        if(url.path() == "delete") slotRemove();
        if(url.path() == "send") emit send( p_sms );
    }
}

void smsPart::slotRemove() { if(p_sms) emit remove( p_sms ); }
void smsPart::slotReply() { if(p_sms && p_sms->isIncoming() ) emit reply( p_sms->getFrom() ); }

void smsPart::show( SMS *sms)
{
    setSMS( sms );
    QString text=getTemplate()
            .arg( KIconLoader::global()->iconPath("smslist", -K3Icon::SizeHuge, false) );
    if ( sms->type() & SMS::Unread || sms->type() & SMS::Read )
    {
        text=text
                .arg( i18n("From: <a href=\"number:%1\" title=\"%2\">%3</a><br>Received Date: %4",
                sms->getFrom(),
                Qt::escape( sms->getFrom() ),
                Qt::escape( KMobiletoolsHelper::translateNumber( sms->getFrom() ) ),
                Qt::escape( sms->getDate() )
                )
                .arg( KMobiletoolsHelper::getFooterLink( i18nc("Reply to this SMS", "Reply"), "mail_reply", "sms:reply" )  )
                .arg( KMobiletoolsHelper::getFooterLink( i18nc("Delete this SMS", "Remove"), "cancel", "sms:delete" )  ) );
    } else
    {
        QString receivers;
        QStringList sl_receivers=sms->getTo();
        QStringList::Iterator it;
        for(it=sl_receivers.begin(); it!=sl_receivers.end(); ++it)
        {
            if( it!=sl_receivers.begin() ) receivers+=", ";
            receivers+=i18n("<a href=\"number:%1\" title=\"%2\">%3</a>",
                    *it,
                    Qt::escape( *it ) ,
                    Qt::escape( KMobiletoolsHelper::translateNumber( *it ) ));
        }
        text=text
                .arg( i18n("To: %1<br>Stored Date: %2",
                    receivers, Qt::escape( sms->getDate()) )
                    );
        if( sms->type() & SMS::Unsent )
            text=text.arg( KMobiletoolsHelper::getFooterLink( i18nc("Send this SMS", "Send"), "mail_send", "sms:send" )  );
        else text=text.arg( KMobiletoolsHelper::getFooterLink( i18nc("Resend this SMS", "Resend"), "mail_send", "sms:send" )  );
        text=text.arg( KMobiletoolsHelper::getFooterLink( i18nc("Delete this SMS", "Remove"), "cancel", "sms:delete" )  );
    }/*
    QString testString="prova ";
    for (uint i=0; i<8; i++) testString+=testString;*/ // Long sms emulation
    text=text.arg( Qt::convertFromPlainText( sms->getText() , Qt::WhiteSpaceNormal) );
//     kDebug() << text << endl;
    begin();
    write(text);
//     kDebug() << "------------------------------ SMS Part ----------------------------------\n";
//     printf( text.latin1());
//     kDebug() << "\n------------------------------ End SMS ----------------------------------\n";
    end();
}

void smsPart::writeHome()
{
    begin();
    write( getTemplate().arg(KIconLoader::global()->iconPath("smslist", -K3Icon::SizeHuge, false) )
            .arg( i18n("%1 SMS List", objectName() ) )
            .arg("")            .arg("")
            .arg( i18n("<p><i>Click a SMS on the list to view it, or \"Reload\" to update the SMS list.</i></p>") )
         );
    end();
    p_sms=0;
}


/*!
    \fn smsPart::popupMenu(const QString &url, const QPoint &point)
 */
void smsPart::slotPopupMenu(const QString &url, const QPoint &point)
{
    if(!p_sms) return;
    KMenu *menu=0;
    KUrl kurl(url);
    if(kurl.protocol()=="number")
    {
        menu=new popupNumber(objectName(), kurl.path(), 0 );
        menu->exec(point);
        return;
    }

    menu=new KMenu ( view() );
    QAction *copy=actionCollection()->addAction(KStandardAction::Copy, "copy", browserExtension(), SLOT(copy()));
    copy->setEnabled( ! selectedText().isNull() );
    menu->addAction(copy);
    if( p_sms->isIncoming())
    {
        QAction *act=actionCollection()->addAction("sms_reply__", this, SLOT(slotReply()));
        act->setText(i18nc("Reply to this SMS", "Reply"));
        act->setIcon(KIcon("mail_reply"));
        menu->addAction(act);
    }
    QAction *act=actionCollection()->addAction("sms_delete__", this, SLOT(slotRemove()));
    act->setText(i18nc("Delete this SMS", "Remove"));
    act->setIcon(KIcon("cancel"));
    menu->addAction(act);

//     menu->insertItem(url);
    menu->exec( point );
}

