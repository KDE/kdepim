/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Barry D Benowitz
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

#include <unistd.h>
#include <stdio.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kprocess.h>

#include <libemailfunctions/email.h>

#include <libkpimidentities/identity.h>
#include <libkpimidentities/identitymanager.h>

#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/incidenceformatter.h>

#include "version.h"
#include "koprefs.h"
#include "kocore.h"

#include "komailclient.h"

KOMailClient::KOMailClient()
{
}

KOMailClient::~KOMailClient()
{
}

bool KOMailClient::mailAttendees(IncidenceBase *incidence,const TQString &attachment)
{
  Attendee::List attendees = incidence->attendees();
  if ( attendees.count() == 0 ) {
    return false;
  }

  const TQString from = incidence->organizer().fullName();
  const TQString organizerEmail = incidence->organizer().email();

  TQStringList toList;
  TQStringList ccList;
  for ( uint i=0; i<attendees.count(); ++i ) {
    Attendee *a = (*attendees.at(i));

    const TQString email = a->email();
    if ( email.isEmpty() ) {
      continue;
    }

    // In case we (as one of our identities) are the organizer we are sending
    // this mail. We could also have added ourselves as an attendee, in which
    // case we don't want to send ourselves a notification mail.
    if ( organizerEmail == email ) {
      continue;
    }

    // Build a nice address for this attendee including the CN.
    TQString tname, temail;
    const TQString username = KPIM::quoteNameIfNecessary( a->name() );
    KPIM::getNameAndMail( username, tname, temail ); // ignore return value
                                                     // which is always false
    tname += " <" + email + '>';


    // Optional Participants and Non-Participants are copied on the email
    if ( a->role() == Attendee::OptParticipant ||
         a->role() == Attendee::NonParticipant ) {
      ccList << tname;
    } else {
      toList << tname;
    }
  }

  if( toList.count() == 0 && ccList.count() == 0 ) {
    // Not really to be called a groupware meeting, eh
    return false;
  }
  TQString to;
  if ( toList.count() > 0 ) {
    to = toList.join( ", " );
  }
  TQString cc;
  if ( ccList.count() > 0 ) {
    cc = ccList.join( ", " );
  }

  TQString subject;
  if(incidence->type()!="FreeBusy") {
    Incidence *inc = static_cast<Incidence *>(incidence);
    subject = inc->summary();
  } else {
    subject = "Free Busy Object";
  }

  TQString body = IncidenceFormatter::mailBodyString(incidence);

  bool bcc = KOPrefs::instance()->mBcc;

  return send(from,to,cc,subject,body,bcc,attachment);
}

bool KOMailClient::mailOrganizer(IncidenceBase *incidence,const TQString &attachment, const TQString &sub)
{
  TQString to = incidence->organizer().fullName();

  TQString from = KOPrefs::instance()->email();

  TQString subject = sub;
  if(incidence->type()!="FreeBusy") {
    Incidence *inc = static_cast<Incidence *>(incidence);
    if ( subject.isEmpty() )
      subject = inc->summary();
  } else {
    subject = "Free Busy Message";
  }

  TQString body = IncidenceFormatter::mailBodyString(incidence);

  bool bcc = KOPrefs::instance()->mBcc;

  return send(from,to,TQString::null,subject,body,bcc,attachment);
}

bool KOMailClient::mailTo(IncidenceBase *incidence,const TQString &recipients,
                          const TQString &attachment)
{
  TQString from = KOPrefs::instance()->email();
  TQString subject;
  if(incidence->type()!="FreeBusy") {
    Incidence *inc = static_cast<Incidence *>(incidence);
    subject = inc->summary();
  } else {
    subject = "Free Busy Message";
  }
  TQString body = IncidenceFormatter::mailBodyString(incidence);
  bool bcc = KOPrefs::instance()->mBcc;
  kdDebug () << "KOMailClient::mailTo " << recipients << endl;
  return send(from,recipients,TQString::null,subject,body,bcc,attachment);
}

bool KOMailClient::send(const TQString &from,const TQString &_to,const TQString &cc,
                        const TQString &subject,const TQString &body,bool bcc,
                        const TQString &attachment)
{
  // We must have a recipients list for most MUAs. Thus, if the 'to' list
  // is empty simply use the 'from' address as the recipient.
  TQString to = _to;
  if ( to.isEmpty() ) {
    to = from;
  }

  kdDebug(5850) << "KOMailClient::sendMail():\nFrom: " << from
                << "\nTo: " << to
                << "\nCC: " << cc
                << "\nSubject: " << subject << "\nBody: \n" << body
                << "\nAttachment:\n" << attachment << endl;

  if (KOPrefs::instance()->mMailClient == KOPrefs::MailClientSendmail) {
    bool needHeaders = true;

    TQString command = KStandardDirs::findExe(TQString::fromLatin1("sendmail"),
        TQString::fromLatin1("/sbin:/usr/sbin:/usr/lib"));
    if (!command.isNull()) command += TQString::fromLatin1(" -oi -t");
    else {
      command = KStandardDirs::findExe(TQString::fromLatin1("mail"));
      if (command.isNull()) return false; // give up

      command.append(TQString::fromLatin1(" -s "));
      command.append(KProcess::quote(subject));

      if (bcc) {
        command.append(TQString::fromLatin1(" -b "));
        command.append(KProcess::quote(from));
      }

      if ( !cc.isEmpty() ) {
        command.append(" -c ");
        command.append(KProcess::quote(cc));
      }

      command.append(" ");
      command.append(KProcess::quote(to));

      needHeaders = false;
    }

    FILE * fd = popen(command.local8Bit(),"w");
    if (!fd)
    {
      kdError() << "Unable to open a pipe to " << command << endl;
      return false;
    }

    TQString textComplete;
    if (needHeaders)
    {
      textComplete += TQString::fromLatin1("From: ") + from + '\n';
      textComplete += TQString::fromLatin1("To: ") + to + '\n';
      if ( !cc.isEmpty() ) {
        textComplete += TQString::fromLatin1("Cc: " ) + cc + '\n';
      }
      if (bcc) textComplete += TQString::fromLatin1("Bcc: ") + from + '\n';
      textComplete += TQString::fromLatin1("Subject: ") + subject + '\n';
      textComplete += TQString::fromLatin1("X-Mailer: KOrganizer") + korgVersion + '\n';
    }
    textComplete += '\n'; // end of headers
    textComplete += body;
    textComplete += '\n';
    textComplete += attachment;

    fwrite(textComplete.local8Bit(),textComplete.length(),1,fd);

    pclose(fd);
  } else {
    if (!kapp->dcopClient()->isApplicationRegistered("kmail")) {
      if (KApplication::startServiceByDesktopName("kmail")) {
        KMessageBox::error(0,i18n("No running instance of KMail found."));
        return false;
      }
    }

    if (attachment.isEmpty()) {
      if (!kMailOpenComposer(to,cc,bcc ? from : "",subject,body,0,KURL())) return false;
    } else {
      TQString meth;
      int idx = attachment.find("METHOD");
      if (idx>=0) {
        idx = attachment.find(':',idx)+1;
        const int newline = attachment.find('\n',idx);
        meth = attachment.mid(idx, newline - idx - 1);
        meth = meth.lower().stripWhiteSpace();
      } else {
        meth = "publish";
      }
      if (!kMailOpenComposer(to,cc,bcc ? from : "",subject,body,0,"cal.ics","7bit",
                             attachment.utf8(),"text","calendar","method",meth,
                             "attachment","utf-8",
                             KOCore::self()->identityManager()->identityForAddress( from ).uoid())) {
        return false;
      }
    }
  }
  return true;
}

int KOMailClient::kMailOpenComposer(const TQString& arg0,const TQString& arg1,
  const TQString& arg2,const TQString& arg3,const TQString& arg4,int arg5,
  const KURL& arg6)
{
  //kdDebug(5850) << "KOMailClient::kMailOpenComposer( "
  //  << arg0 << " , " << arg1 << arg2 << " , " << arg3
  //  << arg4 << " , " << arg5 << " , " << arg6 << " )" << endl;
  int result = 0;

  TQByteArray data, replyData;
  TQCString replyType;
  TQDataStream arg( data, IO_WriteOnly );
  arg << arg0;
  arg << arg1;
  arg << arg2;
  arg << arg3;
  arg << arg4;
  arg << arg5;
  arg << arg6;
#if KDE_IS_VERSION( 3, 2, 90 )
  kapp->updateRemoteUserTimestamp( "kmail" );
#endif
  if (kapp->dcopClient()->call("kmail","KMailIface","openComposer(TQString,TQString,TQString,TQString,TQString,int,KURL)", data, replyType, replyData ) ) {
    if ( replyType == "int" ) {
      TQDataStream _reply_stream( replyData, IO_ReadOnly );
      _reply_stream >> result;
    } else {
      kdDebug(5850) << "kMailOpenComposer() call failed." << endl;
    }
  } else {
    kdDebug(5850) << "kMailOpenComposer() call failed." << endl;
  }
  return result;
}

int KOMailClient::kMailOpenComposer( const TQString& arg0, const TQString& arg1,
                                     const TQString& arg2, const TQString& arg3,
                                     const TQString& arg4, int arg5, const TQString& arg6,
                                     const TQCString& arg7, const TQCString& arg8,
                                     const TQCString& arg9, const TQCString& arg10,
                                     const TQCString& arg11, const TQString& arg12,
                                     const TQCString& arg13, const TQCString& arg14, uint identity )
{
    //kdDebug(5850) << "KOMailClient::kMailOpenComposer( "
    //    << arg0 << " , " << arg1 << arg2 << " , " << arg3
    //   << arg4 << " , " << arg5 << " , " << arg6
    //    << arg7 << " , " << arg8 << " , " << arg9
    //    << arg10<< " , " << arg11<< " , " << arg12
    //    << arg13<< " , " << arg14<< " )" << endl;

    int result = 0;

    TQByteArray data, replyData;
    TQCString replyType;
    TQDataStream arg( data, IO_WriteOnly );
    arg << arg0;
    arg << arg1;
    arg << arg2;
    arg << arg3;
    arg << arg4;
    arg << arg5;
    arg << arg6;
    arg << arg7;
    arg << arg8;
    arg << arg9;
    arg << arg10;
    arg << arg11;
    arg << arg12;
    arg << arg13;
    arg << arg14;
    arg << identity;
#if KDE_IS_VERSION( 3, 2, 90 )
    kapp->updateRemoteUserTimestamp("kmail");
#endif
    if ( kapp->dcopClient()->call("kmail","KMailIface",
          "openComposer(TQString,TQString,TQString,TQString,TQString,int,TQString,TQCString,TQCString,TQCString,TQCString,TQCString,TQString,TQCString,TQCString,uint)", data, replyType, replyData ) ) {
        if ( replyType == "int" ) {
            TQDataStream _reply_stream( replyData, IO_ReadOnly );
            _reply_stream >> result;
        } else {
            kdDebug(5850) << "kMailOpenComposer() call failed." << endl;
        }
    } else {
        kdDebug(5850) << "kMailOpenComposer() call failed." << endl;
    }
    return result;
}


