/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathMailSenderSendmail.h"
#endif

// Qt includes
#include <qcstring.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathMailSenderSendmail.h"
#include "EmpathConfig.h"
#include "Empath.h"

EmpathMailSenderSendmail::EmpathMailSenderSendmail()
    :   EmpathMailSenderImpl(),
        error_(false)
{
    QObject::connect (&sendmailProcess_, SIGNAL(processExited(KProcess *)),
            this, SLOT(sendmailExited(KProcess *)));

    QObject::connect (&sendmailProcess_,
            SIGNAL(receivedStderr(KProcess *, char *, int)),
            this,
            SLOT(sendmailReceivedStderr(KProcess *, char *, int)));

    QObject::connect (&sendmailProcess_, SIGNAL(wroteStdin(KProcess *)),
            this, SLOT(wroteStdin(KProcess *)));
}

EmpathMailSenderSendmail::~EmpathMailSenderSendmail()
{
}

    void
EmpathMailSenderSendmail::setSendmailLocation(const QString & location)
{
    sendmailLocation_ = location;
}

    void
EmpathMailSenderSendmail::sendOne(RMM::RMessage message, const QString & id)
{
    currentID_ = id;
    
    error_ = false;

    empathDebug("Message text:");
    messageAsString_ = message.asString();
    empathDebug(messageAsString_);

    sendmailProcess_.clearArguments();

    KConfig * c = KGlobal::config();

    using namespace EmpathConfig;

    c->setGroup(GROUP_SENDING);

    QString sendmailLocation = c->readEntry(S_SENDMAIL);
    
    if (sendmailLocation.isEmpty()) {
        empathDebug("No location configured for sendmail. Using default");
        sendmailLocation = "/usr/sbin/sendmail";
        c->writeEntry(S_SENDMAIL, sendmailLocation);
    }

    sendmailProcess_ << sendmailLocation;
    sendmailProcess_ << "-t";
    sendmailProcess_ << "-oem";
    sendmailProcess_ << "-oi";

    if (!sendmailProcess_.start(KProcess::NotifyOnExit, KProcess::All)) {
        empathDebug("Couldn't start sendmail process");
        return;
    }

    // Start at first byte of message
    messagePos_ = 0;
    wroteStdin(&sendmailProcess_);
}

    void
EmpathMailSenderSendmail::wroteStdin(KProcess *)
{
    if (messagePos_ >=  messageAsString_.length()) {
        sendmailProcess_.closeStdin();
        written_ = true;
        return;
    }

    int blockSize =
        messagePos_ + 1024 > messageAsString_.length() ?
            messageAsString_.length() - messagePos_ : 1024;
    
    QCString s = messageAsString_.mid(messagePos_, blockSize);

    kapp->processEvents();

    // Remember the current pos in the message string
    messagePos_ += blockSize;
    
    sendmailProcess_.writeStdin((char *)s.data(), s.length());
}

    void
EmpathMailSenderSendmail::sendmailExited(KProcess *)
{
    error_ = (  !sendmailProcess_.normalExit() ||
                sendmailProcess_.exitStatus() != 0);
    
    if (error_) errorStr_ = "sendmail exited abnormally";
    
    written_ = !error_;
    
    messageAsString_ = "";
    
//    sendCompleted(currentID_, !error_);
}

    void
EmpathMailSenderSendmail::sendmailReceivedStderr(
        KProcess *, char * buf, int)
{
    QString eatBuf;
    
    eatBuf = buf + '\n';
    
    // eat
    empathDebug("Process send stderr:");
    empathDebug(eatBuf);
}

    void
EmpathMailSenderSendmail::saveConfig()
{
    KConfig * c = KGlobal::config();
    using namespace EmpathConfig;
    c->setGroup(GROUP_SENDING);
    c->writeEntry(S_SENDMAIL, sendmailLocation_);
}

    void
EmpathMailSenderSendmail::loadConfig()
{
    KConfig * c = KGlobal::config();
    using namespace EmpathConfig;
    c->setGroup(GROUP_SENDING);
    sendmailLocation_ = c->readEntry(S_SENDMAIL, "/usr/lib/sendmail");
}

// vim:ts=4:sw=4:tw=78
