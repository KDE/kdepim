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
# pragma implementation "EmpathMailSender.h"
#endif

// Qt includes
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>

// Local includes
#include "Empath.h"
#include "EmpathEnum.h"
#include "EmpathConfig.h"
#include "EmpathTask.h"
#include "EmpathFolder.h"
#include "EmpathURL.h"
#include "EmpathIndex.h"
#include "EmpathMailSender.h"
#include "EmpathMailSenderSMTP.h"
#include "EmpathMailSenderSendmail.h"
#include "EmpathMailSenderQmail.h"

EmpathMailSender::EmpathMailSender()
    :   QObject(),
        impl_(0L)
{
    sendQueue_.setAutoDelete(true);
    update();
}

EmpathMailSender::~EmpathMailSender()
{
    // Empty.
}

    void
EmpathMailSender::send(RMM::RMessage message)
{
    empath->write(
        message,
        empath->outbox(),
        this,
        SLOT(s_writtenNowSend(EmpathWriteJob)));
}

    void
EmpathMailSender::queue(RMM::RMessage message)
{
    empath->write(
        message,
        empath->outbox(),
        this,
        SLOT(s_writtenNowQueue(EmpathWriteJob)));
}

    void
EmpathMailSender::s_writtenNowSend(EmpathWriteJob job)
{
    if (job.success())
        impl_->sendOne(job.message(), job.messageID());

    else {
        // Warn user that message could not be written to queue
        // folder.
        empathDebug("Couldn't queue message - folder won't accept !");
    
        QMessageBox::critical(0, "Empath",
            i18n("Couldn't queue message ! Writing backup"),
            i18n("OK"));
    
        _emergencyBackup(job.message());
    }
}

    void
EmpathMailSender::s_writtenNowQueue(EmpathWriteJob job)
{
    if (job.success())
        impl_->sendOne(job.message(), job.messageID());

    else {
        // Warn user that message could not be written to queue
        // folder.
        empathDebug("Couldn't queue message - folder won't accept !");
    
        QMessageBox::critical(0, "Empath",
            i18n("Couldn't queue message ! Writing backup"),
            i18n("OK"));
    
        _emergencyBackup(job.message());
    }
}

    void
EmpathMailSender::sendQueued()
{
#if 0
    EmpathFolder * queueFolder(empath->folder(empath->outbox()));

    while (queueFolder->index().count() != 0) {
        sendQueue_.enqueue(new queueFolder->index().at(0));
        _startNextSend();
    }
#endif
}

    void
EmpathMailSender::_startNextSend()
{
    if (sendQueue_.isEmpty())
        return;

    EmpathURL url(empath->outbox());
    
    url.setMessageID(*(sendQueue_.head()));

    empath->retrieve(url, this, SLOT(s_retrievedNowSend(EmpathRetrieveJob)));
}

    void
EmpathMailSender::sendCompleted(const QString & id, bool)
{
    EmpathURL url(empath->outbox());
    url.setMessageID(id);

    empath->move(url, empath->sent(), this, SLOT(s_movedToSent(EmpathMoveJob)));

    sendQueue_.dequeue();
}

    void
EmpathMailSender::s_movedToSent(EmpathMoveJob job)
{
    if (!job.success()) {
        empathDebug("Couldn't move message from queue to sent");
        // TODO Something !
    }
}

     void
EmpathMailSender::_emergencyBackup(RMM::RMessage message)
{
    empathDebug("Writing to emergency backup");

    QString tempName("/tmp/" + empath->generateUnique());

    QFile f(tempName);

    if (!f.open(IO_WriteOnly)) {
        
        empathDebug("Couldn't open the temporary file " + tempName);
        
        empathDebug("EMERGENCY BACKUP COULD NOT BE WRITTEN !");
        
        empathDebug("PLEASE CONTACT PROGRAM MAINTAINER !");
        
        QMessageBox::critical(0, "Empath",
            i18n("Couldn't write the backup file ! Message has been LOST !"),
            i18n("OK"));
        
        return;
    }

    QCString text(message.asString());
    
    f.writeBlock(text.data(), text.length());
    
    f.flush();
    
    f.close();
    
    if (f.status() != IO_Ok) {
        
        empathDebug("Couldn't successfully write the temporary file " +
            tempName);
        
        empathDebug("EMERGENCY BACKUP COULD NOT BE VERIFIED !");
        
        empathDebug("PLEASE CONTACT PROGRAM MAINTAINER !");
        
        QMessageBox::critical(0, "Empath",
        i18n("Couldn't write the backup file ! Message may have been LOST !"),
            i18n("OK"));
        
        return;
    }
    
    QMessageBox::information(0, "Empath",
        i18n("Message backup written to") + " " + tempName,
        i18n("OK"));
}

    void
EmpathMailSender::update()
{
    delete impl_;
    impl_ = 0L;
    
    KConfig * c = KGlobal::config();

    using namespace EmpathConfig;

    c->setGroup(GROUP_GENERAL);
    
    OutgoingServerType st =
        (OutgoingServerType)
        (c->readUnsignedNumEntry(S_TYPE));
    
    switch (st) {
        
        case Qmail:
            impl_ = new EmpathMailSenderQmail;
            break;
        
        case SMTP:
            impl_ = new EmpathMailSenderSMTP;
            break;
            
        case Sendmail:
        default:
            impl_ = new EmpathMailSenderSendmail;
            break;
    }

    impl_->loadConfig();
}

    void
EmpathMailSender::saveConfig()
{ impl_->saveConfig(); }
    
    void
EmpathMailSender::loadConfig()
{ impl_->loadConfig(); }

// vim:ts=4:sw=4:tw=78
