/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
    :    EmpathMailSender(),
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
EmpathMailSenderSendmail::sendOne(RMM::RMessage & message, const QString & id)
{
    empathDebug("sendOne() called");

    currentID_ = id;
    
    error_ = false;

    empathDebug("Message text:");
    messageAsString_ = message.asString();
    empathDebug(messageAsString_);

    sendmailProcess_.clearArguments();

    KConfig * c = KGlobal::config();
    c->setGroup(EmpathConfig::GROUP_SENDING);

    QString sendmailLocation = c->readEntry(EmpathConfig::KEY_SENDMAIL_LOCATION);
    
    if (sendmailLocation.isEmpty()) {
        empathDebug("No location configured for sendmail");
        return;
    }

    empathDebug("sendmail location is" + sendmailLocation);

    sendmailProcess_ << QString(c->readEntry(EmpathConfig::KEY_SENDMAIL_LOCATION));
    sendmailProcess_ << "-t";
    sendmailProcess_ << "-oem";
    sendmailProcess_ << "-oi";

//  Not necessary with -t flag to sendmail
//    sendmailProcess_ << message.recipientListAsPlainString();

    empathDebug("Starting sendmail process");
    if (!sendmailProcess_.start(KProcess::NotifyOnExit, KProcess::All)) {
        empathDebug("Couldn't start sendmail process");
        return;
    }

    empathDebug("Starting piping message to sendmail process");
    
    // Start at first byte of message
    messagePos_ = 0;
    wroteStdin(&sendmailProcess_);
}

    void
EmpathMailSenderSendmail::wroteStdin(KProcess *)
{
    empathDebug("wroteStdin() called");

    if (messagePos_ >=  messageAsString_.length()) {
        empathDebug("messagePos has reached message length");
        sendmailProcess_.closeStdin();
        written_ = true;
        return;
    }

    empathDebug("message pos = " + QString().setNum(messagePos_));

    int blockSize =
        messagePos_ + 1024 > messageAsString_.length() ?
            messageAsString_.length() - messagePos_ : 1024;
    
    QCString s = messageAsString_.mid(messagePos_, blockSize);

    kapp->processEvents();

    empathDebug("Writing \"" + s + "\" to process");

    // Remember the current pos in the message string
    messagePos_ += blockSize;
    
    sendmailProcess_.writeStdin((char *)s.data(), s.length());
}

    void
EmpathMailSenderSendmail::sendmailExited(KProcess *)
{
    empathDebug("Sendmail exited");

    error_ = (  !sendmailProcess_.normalExit() ||
                sendmailProcess_.exitStatus() != 0);
    
    if (error_) errorStr_ = "sendmail exited abnormally";
    
    written_ = !error_;
    
    messageAsString_ = "";
    
    sendCompleted(currentID_, !error_);
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
    c->setGroup(EmpathConfig::GROUP_SENDING);
    c->writeEntry(EmpathConfig::KEY_SENDMAIL_LOCATION, sendmailLocation_);
}

    void
EmpathMailSenderSendmail::readConfig()
{
    KConfig * c = KGlobal::config();
    c->setGroup(EmpathConfig::GROUP_SENDING);
    sendmailLocation_ =
        c->readEntry(EmpathConfig::KEY_SENDMAIL_LOCATION, "/usr/lib/sendmail");
}

// vim:ts=4:sw=4:tw=78
