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
#include "EmpathMailSender.h"
#include "EmpathFolder.h"
#include "EmpathURL.h"
#include "EmpathIndex.h"

EmpathMailSender::EmpathMailSender()
    :   QObject()
{
    sendQueue_.setAutoDelete(true);
}

EmpathMailSender::~EmpathMailSender()
{
    // Empty.
}

    void
EmpathMailSender::send(RMM::RMessage & message)
{
    empath->s_infoMessage(i18n("Sending message"));

    KConfig * c(KGlobal::config());

    using namespace EmpathConfig;

    EmpathURL queueURL(c->readEntry(FOLDER_OUTBOX));
    c->setGroup(GROUP_SENDING);

    empath->write(queueURL, message, "message->pending");
}

    void
EmpathMailSender::queue(RMM::RMessage & message)
{
    KConfig * c(KGlobal::config());

    using namespace EmpathConfig;
    
    c->setGroup(GROUP_SENDING);
    EmpathURL queueURL(c->readEntry(FOLDER_OUTBOX));
    
    using namespace ::std;

    empath->write(queueURL, message, "message->queue");
}

    void
EmpathMailSender::sendQueued()
{
    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_SENDING);
    EmpathURL queueURL(c->readEntry(FOLDER_OUTBOX));
    
    EmpathFolder * queueFolder(empath->folder(queueURL));
    
    if (queueFolder == 0) {
        empathDebug("Couldn't send messages - couldn't find queue folder !");
        return;
    }
    
    QStrList l(queueFolder->index()->allKeys());
    QStrListIterator it(l);
    
    for (; it.current(); ++it) {
        sendQueue_.enqueue(new QString(it.current()));
        if (sendQueue_.count() == 1)
            _startNextSend();
    }
}

    void
EmpathMailSender::_startNextSend()
{
    QString * id = sendQueue_.dequeue();
    
    if (!id) return;
 
    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_SENDING);
    EmpathURL queueURL(c->readEntry(FOLDER_OUTBOX));
    
    EmpathURL url(queueURL);
    
    url.setMessageID(*id);

    empath->retrieve(url, "message->send");
}

    void
EmpathMailSender::sendCompleted(const QString & id, bool)
{
    sendQueue_.dequeue();
    
    if (!sendQueue_.isEmpty())
        _startNextSend();
    
    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;
    EmpathURL queueURL  (c->readEntry(FOLDER_OUTBOX));
    EmpathURL sentURL   (c->readEntry(FOLDER_SENT));

    EmpathURL url(queueURL);
    url.setMessageID(id);

//    empath->copy(url, sentURL, "queue->sent");
}

    void
EmpathMailSender::operationComplete(
    ActionType t, bool b, const EmpathURL & url, QString xinfo)
{
    if ((t == RetrieveMessage) && (xinfo == "message->send")) {
        
        if (b == false) {
            // Warn user that message that should be sent can not be
            // retrieved.
        }

        RMM::RMessage * m(empath->message(url));
 
        if (m == 0) {
        
            empathDebug("Couldn't get a pointer to the next queued message !");
        
            QMessageBox::warning(0, "Empath",
                i18n("Couldn't find next queued message"),
                i18n("OK"));
        
            return;
        }
    
        RMM::RMessage message(*m);
    
        empath->finishedWithMessage(url);

        sendOne(message, url.messageID());

    } else if ((t == WriteMessage) && (xinfo == "message->pending")) {
        
        if (b == false) {
            // Warn user that message could not be written to queue
            // folder.
            empathDebug("Couldn't queue message - folder won't accept !");
        
            QMessageBox::critical(0, "Empath",
                i18n("Couldn't queue message ! Writing backup"),
                i18n("OK"));
        
//                _emergencyBackup(waitingToBeQueued_[url.messageID()]);
        
            empath->s_infoMessage(i18n("Unable to send message"));
        
            return;
        }
        
    } else if ((t == WriteMessage) && (xinfo == "message->queue")) {

        if (b == false) {

            // Warn user that message could not be written to queue
            // folder.
            empathDebug("Couldn't queue message - folder won't accept !");
        
            QMessageBox::critical(0, "Empath",
                i18n("Couldn't queue message ! Writing backup"),
                i18n("OK"));
        
//                _emergencyBackup(waitingToBeQueued_[url.messageID()]);
        
            empath->s_infoMessage(i18n("Unable to send message"));
        
            return;
        }
    }
}

     void
EmpathMailSender::_emergencyBackup(RMM::RMessage & message)
{
    empathDebug("Couldn't queue message ! Writing to emergency backup");

    QString tempName("/tmp/" + empath->generateUnique());

    QFile f(tempName);

    if (!f.open(IO_WriteOnly)) {
        
        empathDebug("Couldn't open the temporary file " + tempName);
        
        empathDebug("EMERGENCY BACKUP COULD NOT BE WRITTEN !");
        
        empathDebug("PLEASE CONTACT PROGRAM MAINTAINER !");
        
        QMessageBox::critical(0, "Empath",
            i18n("Couldn't write the backup file ! Message has been LOST !"),
            i18n("OK"));
        
        empath->s_infoMessage(i18n("Couldn't write backup file !"));
        
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
        
        empath->s_infoMessage(i18n("Couldn't write backup file !"));
        
        return;
    }
    
    QMessageBox::information(0, "Empath",
        i18n("Message backup written to") + " " + tempName,
        i18n("OK"));
    
    empath->s_infoMessage(i18n("Message backup written to:") + " " + tempName);
}

// vim:ts=4:sw=4:tw=78
