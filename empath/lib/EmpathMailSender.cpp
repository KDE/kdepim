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
#include "EmpathConfig.h"
#include "EmpathTask.h"
#include "EmpathMailSender.h"
#include "EmpathFolder.h"
#include "EmpathURL.h"
#include "EmpathIndex.h"


EmpathMailSender::EmpathMailSender()
    :   QObject()
{
    empathDebug("ctor");
    sendQueue_.setAutoDelete(true);
}

EmpathMailSender::~EmpathMailSender()
{
    empathDebug("dtor");
}

    void
EmpathMailSender::send(RMM::RMessage & message)
{
    empath->s_infoMessage(i18n("Sending message"));

    KConfig * c(KGlobal::config());
    c->setGroup(EmpathConfig::GROUP_SENDING);
    
    EmpathURL queueURL  (c->readEntry(EmpathConfig::KEY_QUEUE_FOLDER));
    EmpathURL sentURL   (c->readEntry(EmpathConfig::KEY_SENT_FOLDER));
    
    EmpathFolder * queueFolder(empath->folder(queueURL));
    
    if (queueFolder == 0) {
    
        empathDebug("Couldn't queue message - couldn't find queue folder !");
    
        empathDebug("Queue folder was specified as: \"" +
            queueURL.asString() + "\"");
    
        QMessageBox::critical(0, "Empath",
            i18n("Couldn't queue message ! Writing backup"), i18n("OK"));
    
        _emergencyBackup(message);
    
        empath->s_infoMessage(i18n("Unable to send message"));
    
        return;
    }
    
    QString id(queueFolder->writeMessage(message));
    
    if (id.isNull()) {
    
        empathDebug("Couldn't queue message - folder won't accept !");
    
        QMessageBox::critical(0, "Empath",
            i18n("Couldn't queue message ! Writing backup"),
            i18n("OK"));
    
        _emergencyBackup(message);
    
        empath->s_infoMessage(i18n("Unable to send message"));
    
        return;
    }

    _addPendingSend(id);
}

    void
EmpathMailSender::_addPendingSend(const QString & id)
{
    sendQueue_.enqueue(new QString(id));
    
    if (sendQueue_.count() == 1)
        _startNextSend();
}

    void
EmpathMailSender::_startNextSend()
{
    QString * id = sendQueue_.dequeue();
    
    if (!id) return;
    
    EmpathURL queueURL(
        KGlobal::config()->readEntry(EmpathConfig::KEY_QUEUE_FOLDER));
    
    EmpathURL url(queueURL);
    
    url.setMessageID(*id);

    RMM::RMessage * m(empath->message(url));
 
    if (m == 0) {
        
        empathDebug("Couldn't get a pointer to the next queued message !");
        
        QMessageBox::warning(0, "Empath",
            i18n("Couldn't find next queued message"),
            i18n("OK"));
        
        return;
    }
    
    RMM::RMessage message(*m);
    
    sendOne(message, *id);
}
 
    void
EmpathMailSender::sendCompleted(const QString & id, bool sentOK)
{
    sendQueue_.dequeue();
    
    if (!sendQueue_.isEmpty())
        _startNextSend();
    
    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;
    EmpathURL queueURL  (c->readEntry(KEY_QUEUE_FOLDER));
    EmpathURL sentURL   (c->readEntry(KEY_SENT_FOLDER));
    using namespace std;

    EmpathURL url(queueURL);
    url.setMessageID(id);

    RMM::RMessage * m(empath->message(url));
    
    if (m == 0) {
        
        empathDebug("Couldn't get a pointer to the original message !");
        
        QMessageBox::critical(0, "Empath",
            i18n("Couldn't find queued message to move to sent folder"),
            i18n("OK"));
        
        return;
    }
    
    RMM::RMessage message(*m);
    
    if (!sentOK) {
        
        empathDebug("Couldn't send message !");
        
        QMessageBox::critical(0, "Empath",
            i18n("Couldn't send message ! Writing backup"),
            i18n("OK"));
        
        _emergencyBackup(message);
        
        empath->s_infoMessage(i18n("Unable to send message"));
        
        return;
    }
    
    EmpathFolder * sentFolder(empath->folder(sentURL));
    
    if (sentFolder == 0) {
        
        empathDebug("Can't get a pointer to the sent folder");
        
        QMessageBox::critical(0, "Empath",
            i18n("Please choose a sent mail folder in settings."),
            i18n("OK"));
        
        return;
    }
    
    QString newID(sentFolder->writeMessage(message));

    if (newID.isNull()) {
        
        empathDebug("Couldn't write message to sent folder !");
        
        _emergencyBackup(message);
        
        empath->s_infoMessage(i18n("Unable to send message"));
        
        return;
    }

    EmpathURL q(queueURL);
    q.setMessageID(id);
    
    EmpathFolder * queueFolder(empath->folder(queueURL));
    
    if (!queueFolder->removeMessage(q)) {
        
        empathDebug("Couldn't remove message from queue folder !");
        
        _emergencyBackup(message);
        
        empath->s_infoMessage(i18n("Unable to send message"));
        
        return;
    }
    
    empath->s_infoMessage(i18n("Message sent successfully"));
}

    void
EmpathMailSender::sendQueued()
{
    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_SENDING);
    EmpathURL queueURL(c->readEntry(KEY_QUEUE_FOLDER));
    
    using namespace std;
    
    EmpathFolder * queueFolder(empath->folder(queueURL));
    
    if (queueFolder == 0) {
        empathDebug("Couldn't send messages - couldn't find queue folder !");
        return;
    }
    
    EmpathIndexIterator it(queueFolder->messageList());
    
    for (; it.current(); ++it)
        _addPendingSend(it.current()->id());
}

    void
EmpathMailSender::queue(RMM::RMessage & message)
{
    KConfig * c(KGlobal::config());

    c->setGroup(EmpathConfig::GROUP_SENDING);
    
    EmpathURL queueURL(c->readEntry(EmpathConfig::KEY_QUEUE_FOLDER));
    
    EmpathFolder * queueFolder(empath->folder(queueURL));
    
    if (queueFolder == 0) {
        
        empathDebug("Couldn't queue message - couldn't find queue folder !");
        
        QMessageBox::critical(0, "Empath",
            i18n("Couldn't queue message ! Writing backup"), i18n("OK"));
        
        _emergencyBackup(message);
        
        empath->s_infoMessage(i18n("Unable to queue message"));
        
        return;
    }
    
    if (!queueFolder->writeMessage(message)) {
        
        empathDebug("Couldn't queue message - folder won't accept !");
        
        QMessageBox::critical(0, "Empath",
            i18n("Couldn't queue message ! Writing backup"),
            i18n("OK"));
        
        _emergencyBackup(message);
        
        empath->s_infoMessage(i18n("Unable to queue message"));
        
        return;
    }
    
    empath->s_infoMessage(i18n("Message queued for later delivery"));
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
    
    empath->s_infoMessage(
        i18n("Message backup written to:") + " " + tempName);
}

// vim:ts=4:sw=4:tw=78
