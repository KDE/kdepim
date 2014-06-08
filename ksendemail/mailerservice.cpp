/*
    This file is part of KSendEmail. Some of the code has been taken from KMail (kmkernel.cpp)
    and akonadi (control.h)
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

#include "mailerservice.h"

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include <QtCore/QProcess>

#include <qdebug.h>
#include <kdbusservicestarter.h>
#include <kmessagebox.h>
#include <QUrl>

#include "kmailinterface.h"

static QUrl makeAbsoluteUrl( const QString& str )
{
    QUrl url( str );
    if ( url.scheme().isEmpty() ) {
      const QString newUrl = KCmdLineArgs::cwd() + QLatin1Char('/') + url.fileName();
      return QUrl( newUrl );
    }
    else {
      return url;
    }
}

MailerService::MailerService()
    : mSuccess( false ), mEventLoop( 0 )
{
  connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
           SLOT(serviceOwnerChanged(QString,QString,QString)) );
  start();
}

MailerService::~MailerService()
{

}

bool MailerService::start()
{
  if ( QDBusConnection::sessionBus().interface()->isServiceRegistered( QLatin1String("org.kde.kmail") ) || mEventLoop ) {
    mSuccess = true;
    return mSuccess;
  }
  //Check if Kontact is already running and if not ...
  int result = KDBusServiceStarter::self()->findServiceFor( QLatin1String("DBUS/Mailer"), QString(),
                                                            &mError, &mDBusService );
  if ( result != 0 ) {
    // ... start Kontact
    result = KDBusServiceStarter::self()->startServiceFor( QLatin1String("DBUS/Mailer"), QString(),
                                                           &mError, &mDBusService );
    if(  result != 0 ) {
      const bool ok = QProcess::startDetached( QLatin1String("kontact") );
      if ( !ok ) {
        qWarning() << "Error: unable to execute binary kontact";
        return false;
      }
    } else {
      return false;
    }
  }

  mEventLoop = new QEventLoop( this );
  // safety timeout
  QTimer::singleShot( 10000, mEventLoop, SLOT(quit()) );
  mEventLoop->exec();
  mEventLoop->deleteLater();
  mEventLoop = 0;

  if ( !mSuccess ) {
    qWarning() << "Could not start Mailer Service!";
  }

  return mSuccess;
}

void MailerService::serviceOwnerChanged( const QString & name, const QString & oldOwner, const QString & newOwner )
{
  Q_UNUSED( oldOwner );
  if ( name == QLatin1String("org.kde.kmail") && !newOwner.isEmpty() && mEventLoop && mEventLoop->isRunning() ) {
     mEventLoop->quit();
     mSuccess = true;
  }
}

void MailerService::processArgs( KCmdLineArgs *args )
{
    QString to, cc, bcc, subj, body;
    QStringList customHeaders;
    QUrl messageFile;
    QStringList attachURLs;
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
        if ( subj == QLatin1String("ession") ) {
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
        const QString file = args->getOption("msg");
        messageFile = makeAbsoluteUrl(file);
    }

    if (args->isSet("body"))
    {
        mailto = true;
        body = args->getOption("body");
    }

    const QStringList attachList = args->getOptionList("attach");
    if (!attachList.isEmpty())
    {
        mailto = true;
        QStringList::ConstIterator end(attachList.constEnd());
        for ( QStringList::ConstIterator it = attachList.constBegin() ; it != end ; ++it )
        {
            if ( !(*it).isEmpty() ) {
                attachURLs.append( makeAbsoluteUrl( *it ).url() );
            }
        }
    }

    customHeaders = args->getOptionList("header");

    if (args->isSet("composer"))
        mailto = true;


    if ( !calledWithSession ) {
    // only read additional command line arguments if kmail/kontact is
    // not called with "-session foo"
        const int numberOfArgs(args->count());
        for(int i= 0; i < numberOfArgs; ++i) {
            if (args->arg(i).startsWith(QLatin1String("mailto:"), Qt::CaseInsensitive)) {
                to += args->url(i).path() + QLatin1String(", ");
            } else {
                const QString tmpArg = args->arg(i);
                QUrl url( tmpArg );
                if (url.isValid() && !url.scheme().isEmpty())
                    attachURLs.append( url.url() );
                else
                    to += tmpArg + QLatin1String(", ");
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

    if( !mailto )
       return;
    if ( mSuccess ) {
     QDBusInterface kmailObj( QLatin1String("org.kde.kmail"), QLatin1String("/KMail"), QLatin1String("org.kde.kmail.kmail") );

     QList<QVariant> messages;
     messages << to << cc << bcc << subj << body << false << messageFile.url() << attachURLs << customHeaders;
     QDBusReply<int> composerDbusPath = kmailObj.callWithArgumentList(QDBus::AutoDetect, QLatin1String("openComposer"), messages);

     if ( !composerDbusPath.isValid() ) {
      KMessageBox::error( 0, i18n( "Cannot connect to email service." ) );
     }
    } else {
      KMessageBox::error( 0, i18n( "Unable to find or start email service." ) );
    }

}

