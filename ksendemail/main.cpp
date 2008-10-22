/*
    This file is part of KSendEmail. Some of the code has been taken from KMail (kmkernel.cpp)
    Copyright (c) 2008 Pradeepto Bhattacharya <pradeepto@kde.org>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtDBus/QtDBus>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kurl.h>
#include <kdebug.h>
#include <kdbusservicestarter.h>
#include <kmessagebox.h>

#include "kmailinterface.h"

static const char description[] =
  I18N_NOOP( "KDE Commandline Emailer." );

static KCmdLineOptions kmail_options ()
{
    KCmdLineOptions options;
    options.add("s");
    options.add("subject <subject>",	ki18n("Set subject of message"));
    options.add("c");
    options.add("cc <address>",		ki18n("Send CC: to 'address'"));
    options.add("b");
    options.add("bcc <address>",		ki18n("Send BCC: to 'address'"));
    options.add("h");
    options.add("header <header>",	ki18n("Add 'header' to message"));
    options.add("msg <file>",		ki18n("Read message body from 'file'"));
    options.add("body <text>",		ki18n("Set body of message"));
    options.add("attach <url>",		ki18n("Add an attachment to the mail. This can be repeated"));
    options.add("composer",		ki18n("Only open composer window"));
    options.add("+[address|URL]",		ki18n("Send message to 'address' resp. "
            "attach the file the 'URL' points "
                    "to"));
    return options;
}


void processArgs( KCmdLineArgs *args )
{
    kDebug() << "bool processArgs( KCmdLineArgs *args )";

    QString to, cc, bcc, subj, body;
    QStringList customHeaders;
    KUrl messageFile;
    KUrl::List attachURLs;
    bool mailto = false;
    bool calledWithSession = false; // for ignoring '-session foo'

    if (args->isSet("subject"))
    {
        subj = args->getOption("subject");
     // if kmail is called with 'kmail -session abc' then this doesn't mean
     // that the user wants to send a message with subject "ession" but
     // (most likely) that the user clicked on KMail's system tray applet
     // which results in KMKernel::raise() calling "kmail kmail newInstance"
     // via D-Bus which apparently executes the application with the original
     // command line arguments and those include "-session ..." if
     // kmail/kontact was restored by session management
        if ( subj == "ession" ) {
            subj.clear();
            calledWithSession = true;
        }
        else
            mailto = true;
    }

    if (args->isSet("cc"))
    {
        mailto = true;
        cc = args->getOption("cc");
    }

    if (args->isSet("bcc"))
    {
        mailto = true;
        bcc = args->getOption("bcc");
    }

    if (args->isSet("msg"))
    {
        mailto = true;
        messageFile.setPath( args->getOption("msg") );
    }

    if (args->isSet("body"))
    {
        mailto = true;
        body = args->getOption("body");
    }

    QStringList attachList = args->getOptionList("attach");
    if (!attachList.isEmpty())
    {
        mailto = true;
        for ( QStringList::Iterator it = attachList.begin() ; it != attachList.end() ; ++it )
            if ( !(*it).isEmpty() )
                attachURLs += KUrl( *it );
    }

    customHeaders = args->getOptionList("header");

    if (args->isSet("composer"))
        mailto = true;


    if ( !calledWithSession ) {
    // only read additional command line arguments if kmail/kontact is
    // not called with "-session foo"
        for(int i= 0; i < args->count(); i++)
        {
            if (args->arg(i).startsWith("mailto:", Qt::CaseInsensitive))
                to += args->url(i).path() + ", ";
            else {
                QString tmpArg = args->arg(i);
                KUrl url( tmpArg );
                if (url.isValid() && !url.protocol().isEmpty())
                    attachURLs += url;
                else
                    to += tmpArg + ", ";
            }
            mailto = true;
        }
        if ( !to.isEmpty() ) {
      // cut off the superfluous trailing ", "
            to.truncate( to.length() - 2 );
        }
    }

    if ( !calledWithSession )
        args->clear();

    QString error;
    QString dbusService;
    int result = KDBusServiceStarter::self()->findServiceFor( "DBUS/ResourceBackend/IMAP", QString(), &error, &dbusService );//Check if Kontact is already running and if not ...
    if ( result != 0 ) { 
      result = KDBusServiceStarter::self()->startServiceFor( "DBUS/ResourceBackend/IMAP", QString(), &error, &dbusService ); // ... start Kontact
      if(  result != 0 ) {
        KMessageBox::error( 0, i18n( "Unable to find or start email service." ) );
        return;
      }
    }

    QDBusInterface kmailObj( dbusService, "/KMail", "org.kde.kmail.kmail" );
    QList<QVariant> messages;
    messages << to << cc << bcc << subj << body << false;
    QDBusReply<QDBusObjectPath> composerDbusPath = kmailObj.callWithArgumentList(QDBus::AutoDetect, "openComposer", messages);

    if ( !composerDbusPath.isValid() ) {
      KMessageBox::error( 0, i18n( "Can't connect to email service." ) );
    }
}

int main( int argc, char **argv )
{
  KAboutData aboutData( "ksendemail", 0, ki18n( "KSendEmail" ), "0.01", ki18n(description),
                      KAboutData::License_GPL,
                      ki18n( "(C) 2008 Pradeepto Bhattacharya" ),
                             KLocalizedString(), "http://kontact.kde.org" );

  aboutData.addAuthor( ki18n( "Pradeepto Bhattacharya" ), KLocalizedString(), "pradeepto@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( kmail_options() );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  processArgs( args );
}
