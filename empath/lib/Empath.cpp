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
# pragma implementation "Empath.h"
#endif

// System includes
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>

// Qt includes
#include <qtimer.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathMailboxList.h"
#include "EmpathIndexRecord.h"
#include "EmpathMessageList.h"
#include "EmpathMailSenderSendmail.h"
#include "EmpathMailSenderQmail.h"
#include "EmpathMailSenderSMTP.h"
#include "EmpathFilterList.h"
#include "EmpathTask.h"
#include "EmpathTaskTimer.h"

Empath * Empath::EMPATH = 0;
bool Empath::started_ = false;

    void
Empath::start()
{
    if (!started_) {
        started_ = true;
        new Empath;
    }
}
        
    void
Empath::shutdown()
{
    empathDebug("dtor");

    filterList_.save();
    mailboxList_.saveConfig();
    KGlobal::config()->sync();

    delete mailSender_;
    mailSender_ = 0;

    delete this;
}

Empath::Empath()
    :   QObject(),
        mailSender_(0),
        seq_(0)
{
    empathDebug("ctor");
    EMPATH = this;
    cache_.setMaxCost(1048576);        // 1Mb cache
    updateOutgoingServer();            // Must initialise the pointer.
}

    void
Empath::init()
{
    empathDebug("init() called");
    processID_ = getpid();
    pidStr_.setNum(processID_);
    _saveHostName();
    _setStartTime();
    mailboxList_.init();
    filterList_.load();

    QString userName = KGlobal::config()->readEntry(EmpathConfig::KEY_NAME);
    if (!userName)
        emit(setupWizard());
}

Empath::~Empath()
{
}

    void
Empath::s_saveConfig()
{
    filterList_.save();
    mailboxList_.saveConfig();
    KGlobal::config()->sync();
}

    void
Empath::updateOutgoingServer()
{
    delete mailSender_;
    mailSender_ = 0;
    
    KConfig * c = KGlobal::config();
    c->setGroup(EmpathConfig::GROUP_GENERAL);
    
    EmpathMailSender::OutgoingServerType st =
        (EmpathMailSender::OutgoingServerType)
        (c->readUnsignedNumEntry(EmpathConfig::KEY_OUTGOING_SERVER_TYPE,
                                 EmpathMailSender::Sendmail));
    
    switch (st) {
        
        case EmpathMailSender::Sendmail:
            mailSender_ = new EmpathMailSenderSendmail;
            break;
        
        case EmpathMailSender::Qmail:
            mailSender_ = new EmpathMailSenderQmail;
            break;
        
        case EmpathMailSender::SMTP:
            mailSender_ = new EmpathMailSenderSMTP;
            break;
            
        default:
            mailSender_ = 0;
            return;
            break;
    }

    CHECK_PTR(mailSender_);

    mailSender_->readConfig();
}

    RMM::RMessage *
Empath::message(const EmpathURL & source)
{
    empathDebug("message(" + source.asString() + ") called");
    
    // Try and get the message from the cache.
    
    RMM::RMessage * message(cache_[source.messageID()]);
    
    if (message != 0) {
        empathDebug("message \"" + source.asString() + "\" found in cache");
        return message;
    }
    
    empathDebug("message \"" + source.asString() + "\" not in cache");
    
    // It's not in the cache. We don't have another source these days, due
    // to the async code.
    return 0;
}

    EmpathMailbox *
Empath::mailbox(const EmpathURL & url)
{
    EmpathMailboxListIterator it(mailboxList_);
    
    for (; it.current(); ++it)
        if (it.current()->name() == url.mailboxName())
            return it.current();
    
    return 0;
}

    EmpathFolder *
Empath::folder(const EmpathURL & url)
{
    empathDebug("folder(" + url.asString() + ") called");
    EmpathMailbox * m = mailbox(url);
    if (m == 0) {
        empathDebug("Can't find mailbox " + url.asString());
        return 0;
    }
    return m->folder(url);
}

    void
Empath::copy(const EmpathURL & from, const EmpathURL & to, QString xinfo)
{
    EmpathMailbox * m_from = mailbox(from);
    
    if (m_from == 0) {
        empathDebug("Can't find mailbox " + from.mailboxName());
        emit(copyComplete(false, from, to, xinfo));
        return;
    }

    m_from->retrieve(from, to, "copy", xinfo);
}

    void
Empath::move(const EmpathURL & from, const EmpathURL & to, QString xinfo)
{
    EmpathMailbox * m_from = mailbox(from);
    
    if (m_from == 0) {
        empathDebug("Can't find mailbox " + from.mailboxName());
        emit(moveComplete(false, from, to, xinfo));
        return;
    }

    m_from->retrieve(from, to, "move", xinfo);
}

    void
Empath::retrieve(const EmpathURL & url, QString xinfo)
{
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0) {
        empathDebug("Can't find mailbox " + url.mailboxName());
        emit(retrieveComplete(false, url, xinfo));
        return;
    }

    m->retrieve(url, xinfo);
}

    EmpathURL
Empath::write(const EmpathURL & url, RMM::RMessage & msg, QString xinfo)
{
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0) {
        empathDebug("Can't find mailbox " + url.mailboxName());
        emit(writeComplete(false, url, xinfo));
        return EmpathURL("");
    }
    return m->write(url, msg, xinfo);
}

    void
Empath::createFolder(const EmpathURL & url, QString xinfo)
{
    EmpathMailbox * m = mailbox(url);

    if (m == 0) {
        empathDebug("Can't find mailbox " + url.mailboxName());
        emit(createFolderComplete(false, url, xinfo));
    }

    m->createFolder(url, xinfo);
}

    void
Empath::remove(const EmpathURL & url, QString xinfo)
{
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0) {
        empathDebug("Can't find mailbox " + url.mailboxName());
        if (url.hasMessageID())
            emit(removeComplete(false, url, xinfo));
        else
            emit(removeComplete(false, url, xinfo));
        return;
    }
    m->remove(url, xinfo);
}

    void
Empath::remove(const EmpathURL & url, const QStringList & l, QString xinfo)
{
    EmpathMailbox * m = mailbox(url);

    if (m == 0) {
        empathDebug("Can't find mailbox " + url.mailboxName());
        if (url.hasMessageID())
            emit(removeComplete(false, url, xinfo));
        else
            emit(removeComplete(false, url, xinfo));
        return;
    }

    return m->remove(url, l, xinfo);
}

    void
Empath::mark(const EmpathURL & url, RMM::MessageStatus s, QString xinfo)
{
    empathDebug("mark() called");
    
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0) {
        empathDebug("Can't find mailbox " + url.mailboxName());
        emit(markComplete(false, url, xinfo));
        return;
    }

    m->mark(url, s, xinfo);
}

    void
Empath::mark(
    const EmpathURL & url,
    const QStringList & l,
    RMM::MessageStatus s,
    QString xinfo)
{
    empathDebug("mark() called");

    EmpathMailbox * m = mailbox(url);
    
    if (m == 0) {
        empathDebug("Can't find mailbox " + url.mailboxName());
        emit(markComplete(false, url, xinfo));
        return;
    }

    m->mark(url, l, s, xinfo);
}

    EmpathTask *
Empath::addTask(const QString & name)
{
    EmpathTask * t = new EmpathTask(name);
    CHECK_PTR(t);
    new EmpathTaskTimer(t);
    return t;
}


// Private methods follow

    void
Empath::_setStartTime()
{
    struct timeval timeVal;
    struct timezone timeZone;
    
    gettimeofday(&timeVal, &timeZone);
    startupSeconds_ = timeVal.tv_sec;
    startupSecondsStr_.setNum(startupSeconds_);
}
    void
Empath::_saveHostName()
{
    struct utsname utsName;
    if (uname(&utsName) == 0)
        hostName_ = utsName.nodename;
}

    QString
Empath::generateUnique()
{
    QString unique;

    unique =
        startupSecondsStr_ +
        '.' +
        pidStr_ +
        '_' +
        QString().setNum(seq_) +
        '.' +
        hostName_;

    ++seq_;

    return unique;
}

    void
Empath::cacheMessage(const EmpathURL & url, RMM::RMessage * m)
{
    cache_.insert(url.messageID(), m);
}

    void
Empath::s_retrieveComplete(
    bool status,
    const EmpathURL & from,
    const EmpathURL & to,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(retrieveComplete(status, from, to, xinfo));
}

    void
Empath::s_retrieveComplete(
    bool status,
    const EmpathURL & url,
    QString xinfo)
{
    emit(retrieveComplete(status, url, xinfo));
}

    void
Empath::s_moveComplete(
    bool status,
    const EmpathURL & from,
    const EmpathURL & to,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(moveComplete(status, from, to, xinfo));
}

    void
Empath::s_copyComplete(
    bool status,
    const EmpathURL & from,
    const EmpathURL & to,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(copyComplete(status, from, to, xinfo));
}

    void
Empath::s_removeComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(removeComplete(status, url, xinfo));
}

    void
Empath::s_markComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(markComplete(status, url, xinfo));
}

    void
Empath::s_writeComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo)
{    
    emit(writeComplete(status, url, xinfo));
}

    void
Empath::s_createFolderComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo) 
{
    emit(createFolderComplete(status, url, xinfo));
}

    void
Empath::s_removeFolderComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(removeFolderComplete(status, url, xinfo));
}

    void
Empath::saveMessage(const EmpathURL & url)
{
#if 0
    QString saveFilePath =
        KFileDialog::getSaveFileName(
            QString::null, QString::null, this,
            i18n("Empath: Save Message").ascii());
    empathDebug(saveFilePath);
    
    if (saveFilePath.isEmpty()) {
        empathDebug("No filename given");
        return;
    }
    
    QFile f(saveFilePath);
    if (!f.open(IO_WriteOnly)) {
        // Warn user file cannot be opened.
        empathDebug("Couldn't open file for writing");
        QMessageBox::information(this, "Empath",
            i18n("Sorry I can't write to that file. "
                "Please try another filename."), i18n("OK"));
        return;
    }
    empathDebug("Opened " + saveFilePath + " OK");
    
    // FIXME: Here we should ask for the message, wait for the signal,
    // then do the actual writing.
    
    QCString s =
        message->asString();
    
    unsigned int blockSize = 1024; // 1k blocks
    
    unsigned int fileLength = s.length();

    for (unsigned int i = 0 ; i < s.length() ; i += blockSize) {
        
        QCString outStr;
        
        if ((fileLength - i) < blockSize)
            outStr = QCString(s.right(fileLength - i));
        else
            outStr = QCString(s.mid(i, blockSize));
        
        if (f.writeBlock(outStr, outStr.length()) != (int)outStr.length()) {
            // Warn user file not written.
            QMessageBox::information(this, "Empath",
                i18n("Sorry I couldn't write the file successfully. "
                    "Please try another file."), i18n("OK"));
            delete message; message = 0;
            return;
        }
        qApp->processEvents();
    }

    f.close();
    
    QMessageBox::information(this, "Empath",
        i18n("Message saved to") + " " + saveFilePath + " " + i18n("OK"),
        i18n("OK"));
    delete message; message = 0;
#endif

}

// vim:ts=4:sw=4:tw=78
