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


// Qt includes
#include <qcstring.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <kapplication.h>

// Local includes
#include "EmpathMailSenderQmail.h"
#include "Empath.h"

EmpathMailSenderQmail::EmpathMailSenderQmail()
    :   EmpathMailSenderImpl(),
        error_(false)
{
    QObject::connect (&qmailProcess_, SIGNAL(processExited(KProcess *)),
            this, SLOT(qmailExited(KProcess *)));

    QObject::connect (&qmailProcess_,
            SIGNAL(receivedStderr(KProcess *, char *, int)),
            this,
            SLOT(qmailReceivedStderr(KProcess *, char *, int)));

    QObject::connect (&qmailProcess_, SIGNAL(wroteStdin(KProcess *)),
            this, SLOT(wroteStdin(KProcess *)));
}

EmpathMailSenderQmail::~EmpathMailSenderQmail()
{
}

    void
EmpathMailSenderQmail::setQmailLocation(const QString & location)
{
    qmailLocation_ = location;
}

    void
EmpathMailSenderQmail::sendOne(RMM::Message message, const QString & id)
{
    currentID_ = id;

    error_ = false;

    messageAsString_ = message.asString();
    empathDebug(messageAsString_);

    qmailProcess_.clearArguments();

    KConfig * c = KGlobal::config();

    c->setGroup("Sending");

    qmailProcess_ << c->readEntry("QmailLocation");

    if (!qmailProcess_.start(KProcess::NotifyOnExit, KProcess::All)) {
        empathDebug("Couldn't start qmail process");
        return;
    }

    messagePos_ = 0;
    wroteStdin(&qmailProcess_);
}

    void
EmpathMailSenderQmail::wroteStdin(KProcess *)
{
    empathDebug("wroteStdin() called");

    if (messagePos_ >=  messageAsString_.length()) {
        empathDebug("messagePos has reached message length");
        qmailProcess_.closeStdin();
        written_ = true;
        return;
    }

    empathDebug("message pos = " + QString().setNum(messagePos_));

    int blockSize =
        messagePos_ + 1024 > messageAsString_.length() ?
            messageAsString_.length() - messagePos_ : 1024;

    QCString s = messageAsString_.mid(messagePos_, blockSize);

    empathDebug("Writing \"" + s + "\" to process");

    messagePos_ += blockSize;

    qmailProcess_.writeStdin((char *)s.data(), s.length());
}

    void
EmpathMailSenderQmail::qmailExited(KProcess *)
{
    empathDebug("qmail exited");

    error_ = (!qmailProcess_.normalExit() || qmailProcess_.exitStatus() != 0);

    if (error_) errorStr_ = "qmail exited abnormally";

    written_ = !error_;

    //sendCompleted(currentID_, !error_);
}

    void
EmpathMailSenderQmail::qmailReceivedStderr(KProcess *, char * buf, int)
{
    QString eatBuf;

    eatBuf = buf + '\n';

    // eat
    empathDebug("Process send stderr:");
    empathDebug(eatBuf);
}

    void
EmpathMailSenderQmail::saveConfig()
{
    KConfig * c = KGlobal::config();
    c->setGroup("Sending");
    c->writeEntry("QmailLocation", qmailLocation_);
}

    void
EmpathMailSenderQmail::loadConfig()
{
    KConfig * c = KGlobal::config();
    c->setGroup("Sending");
    qmailLocation_ = c->readEntry("QmailLocation", "/var/qmail/bin/qmail-inject");
}

// vim:ts=4:sw=4:tw=78
#include "EmpathMailSenderQmail.moc"
