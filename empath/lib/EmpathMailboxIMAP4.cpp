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

// System includes
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

// Qt includes
#include <qsocketdevice.h>

// Local includes
#include "IMAPClient.h"
#include "EmpathMailboxIMAP4.h"
#include "rmm/Envelope.h"

EmpathMailboxIMAP4::EmpathMailboxIMAP4(const QString & name)
    :   EmpathMailbox   (name),
        client_         (0),
        serverAddress_  ("localhost"),
        serverPort_     (143),
        username_       (QString::null),
        password_       (QString::null)
{
    type_ = IMAP4;
    typeString_ = "IMAP4";
}

EmpathMailboxIMAP4::~EmpathMailboxIMAP4()
{
    // Empty.
}

    void
EmpathMailboxIMAP4::s_checkMail()
{
    empathDebug("STUB");
}

    void
EmpathMailboxIMAP4::saveConfig()
{
    empathDebug("STUB");
}

    void
EmpathMailboxIMAP4::loadConfig()
{
    empathDebug("STUB");
}

// Set methods

    void
EmpathMailboxIMAP4::setServerAddress(const QString & serverAddress)
{
    serverAddress_    = serverAddress;
}

    void
EmpathMailboxIMAP4::setServerPort(Q_UINT32 serverPort)
{
    serverPort_ = serverPort;
}

    void
EmpathMailboxIMAP4::setUsername(const QString & username)
{
    username_ = username;
}

    void
EmpathMailboxIMAP4::setPassword(const QString & password)
{
    password_ = password;
}

// Get methods

    QString
EmpathMailboxIMAP4::serverAddress()
{
    return serverAddress_;
}

    Q_UINT32
EmpathMailboxIMAP4::serverPort()
{
    return serverPort_;
}

    QString
EmpathMailboxIMAP4::username()
{
    return username_;
}

    QString
EmpathMailboxIMAP4::password()
{
    return password_;
}

    bool
EmpathMailboxIMAP4::newMail() const
{
    empathDebug("STUB");
    return false;
}
    void
EmpathMailboxIMAP4::sync(const EmpathURL & url)
{
    empathDebug(url.asString());
    EmpathIndex * i = 0;

    for (QListIterator<EmpathIndex> it(indexList_); it.current(); ++it)
        if (it.current()->folder().folderPath() == url.folderPath()) {
            i = it.current();
            break;
        }

    if (0 == i) {
        empathDebug("Can't sync. No index.");
        return;
    }

    i->clear();

    bool ok = false;

    IMAPClient::MailboxInfo info;

    ok = client_->selectMailbox(url.folderPath(), info);

    if (!ok) {
        empathDebug("Couldn't select folder " + url.folderPath());
        return;
    }

    if (!info.countAvailable()) {
        empathDebug("Mailbox info doesn't have message count");
        return;
    }

    empathDebug("Running FETCH...");
    QStringList l = client_->fetch(1, info.count(),
            "(UID FLAGS RFC822.SIZE BODY[HEADER.FIELDS (SUBJECT SENDER FROM DATE MESSAGE-ID REFERENCES IN-REPLY-TO CONTENT-TYPE)])");
    empathDebug("FETCH complete");

    /* We should have lots of sections like this:

1365 FETCH (FLAGS (\Seen) RFC822.SIZE 2919 BODY[HEADER.FIELDS ("SUBJECT" "SENDER" "FROM" "DATE" "MESSAGE-ID" "REFERENCES" "IN-REPLY-TO" "CONTENT-TYPE")] {269}
Date: Sun, 03 Sep 2000 03:51:39 +0200
From: Stephan Kulow <coolo@kde.org>
Subject: Re: still testing
References: <39B1A77D.5DA85342@kde.org> <00090218361605.23435@wantelbos>
Content-Type: text/plain; charset=us-ascii
Sender: kde-core-devel-admin@master.kde.org

        ) 
    */

    for (QStringList::ConstIterator it(l.begin()); it != l.end(); ++it) {

        QString line = *it;

        if (line[0] == '*') {

            QStringList fetchReply;
            QStringList::ConstIterator it2(it);

            for (; it2 != l.end() && (*it2)[0] != ')'; ++it2)
                fetchReply << *it2;

            EmpathIndexRecord * rec =
                _createIndexRecordFromFetchReply(fetchReply);

            if (0 != rec) {
                i->insert(rec->id(), *rec);
            } else {
                empathDebug("Couldn't create index record from fetch reply");
            }
        }
    }
}

    void
EmpathMailboxIMAP4::init()
{
    empathDebug("init() called");

    folderList_.clear();

    username_ = "rik";
    password_ = "poker";

    empathDebug("Cleared folder list");

    int fd = -1;

    struct sockaddr_in sin;
    struct hostent * he;

    ::memset(&sin, 0, sizeof(sin));

    sin.sin_port = htons(serverPort_);
    sin.sin_family = AF_INET;

    empathDebug("Doing gethostbyname");

    he = ::gethostbyname(serverAddress_.ascii());

    if (NULL == he) {
        empathDebug("Couldn't find host");
        return;
    }

    for (int i = 0; he ->h_addr_list[i] != NULL; i++)
    {
        ::memcpy(&sin.sin_addr, he->h_addr_list[i], he->h_length);
        fd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_IP);

        if (fd >= 0)
        {
            int x = ::connect(
                    fd,
                    (struct sockaddr *)(&sin),
                    sizeof(struct sockaddr_in)
            );

            if (0 == x) {
                empathDebug("Connected");
                break;
            }

            ::close(fd);
            fd = -1;
        }
    }

    if (-1 == fd) {
        empathDebug("Couldn't connect to host");
        return;
    }
    
    empathDebug("Connected");

    socket_ = new QSocketDevice(fd, QSocketDevice::Stream);
    socket_->setBlocking(true);

    client_ = new IMAPClient(socket_);

    bool ok = client_->login(username_, password_);

    if (!ok) {
        empathDebug("Login failed");
        return;
    }

    QValueList<IMAPClient::ListResponse> l;

    ok = client_->list("", "*", l);

    if (!ok) {
        empathDebug("LIST failed");
        return;
    }

    QValueList<IMAPClient::ListResponse>::ConstIterator it(l.begin());

    for (; it != l.end(); ++it) {
        QString folderName = (*it).name();
        EmpathURL folderURL("empath://" + name() + "/" + folderName + "/");
        empathDebug("Adding folder with url `" + folderURL.asString() + "'");
        folderList_.insert(folderName, new EmpathFolder(folderURL));
        indexList_.append(new EmpathIndex(folderURL));
    }

    emit(updateFolderLists());
}

    bool
EmpathMailboxIMAP4::removeMessage(const EmpathURL & url)
{
    bool ok = false;

    IMAPClient::MailboxInfo info;

    ok = client_->selectMailbox(url.folderPath(), info);

    if (!ok) {
        empathDebug("Couldn't select folder " + url.folderPath());
        return false;
    }

    ok = client_->setFlags(
            url.messageID().toULong(),
            url.messageID().toULong(),
            IMAPClient::Add,
            IMAPClient::Deleted,
            true
    );

    if (!ok) {
        empathDebug("Couldn't set deleted flag on " + url.messageID());

        return false;
    }

    return true;
}

    bool
EmpathMailboxIMAP4::createFolder(const EmpathURL & url)
{
    return client_->createMailbox(url.folderPath());
}

    bool
EmpathMailboxIMAP4::removeFolder(const EmpathURL & url)
{
    return client_->removeMailbox(url.folderPath());
}

    RMM::Message
EmpathMailboxIMAP4::retrieveMessage(const EmpathURL & url)
{
    RMM::Message retval;

    bool ok = false;

    IMAPClient::MailboxInfo info;

    ok = client_->selectMailbox(url.folderPath(), info);

    if (!ok) {
        empathDebug("Couldn't select folder " + url.folderPath());
        return retval;
    }

    ulong uid = url.messageID().toULong();

    QStringList body = client_->fetch(uid, uid, "RFC822", true);

    if (body.count() == 0) {
        empathDebug("Duh.. body count is 0");
        return retval;
    }

    QString data;

    for (QStringList::ConstIterator it(body.begin()); it != body.end(); ++it)
        data += (*it) + '\n';

    retval = RMM::Message(data.ascii());

    return retval;
}

    QString
EmpathMailboxIMAP4::writeMessage(
    RMM::Message &,
    const EmpathURL &
)
{
    empathDebug("STUB");
    return QString::null;
}

    bool
EmpathMailboxIMAP4::markMessage(const EmpathURL &, EmpathIndexRecord::Status)
{
    empathDebug("STUB");
    return false;
}

    EmpathSuccessMap
EmpathMailboxIMAP4::markMessage(
    const EmpathURL &,
    const QStringList &,
    EmpathIndexRecord::Status)
{
    empathDebug("STUB");
    EmpathSuccessMap retval;
    return retval;
}

    EmpathSuccessMap
EmpathMailboxIMAP4::removeMessage(
    const EmpathURL & /* folder */,
    const QStringList & /* messageIDList */
)
{
    empathDebug("STUB");
    EmpathSuccessMap retval;
    return retval;
}

    unsigned int
EmpathMailboxIMAP4::messageCount() const
{
    empathDebug("STUB");
    return 0;
}

    unsigned int
EmpathMailboxIMAP4::unreadMessageCount() const
{
    empathDebug("STUB");
    return 0;
}

    EmpathIndex *
EmpathMailboxIMAP4::index(const EmpathURL & url)
{
    sync(url);
    empathDebug("Looking for `" + url.folderPath() + "'");
    for (QListIterator<EmpathIndex> it(indexList_); it.current(); ++it) {
        empathDebug("Looking at `" + it.current()->folder().folderPath() + "'");
        if (it.current()->folder().folderPath() == url.folderPath()) {
            return it.current();
        }
    }

    return 0;
}

    EmpathIndexRecord *
EmpathMailboxIMAP4::_createIndexRecordFromFetchReply(const QStringList & l)
{
    QString line = *(l.begin());

    QString subject, senderName, senderAddress, messageID, parentID;
    QDateTime date;
    int timeZone = 0;
    EmpathIndexRecord::Status status = 0;
    uint size;
    bool hasAttachments = false;

    int flagsPos = line.find("FLAGS");

    if (-1 == flagsPos) {
        empathDebug("No FLAGS section in string. Weird.");
        return 0;
    } else {

        int flagsStart = line.find('(', flagsPos);
        int flagsEnd = line.find(')', flagsStart);

        if (
                (-1 == flagsStart) ||
                (-1 == flagsEnd) ||
                (flagsStart >= flagsEnd)
           )
        {
            empathDebug("Couldn't find flags");
            return 0;

        } else {

            QString flags =
                line.mid(flagsStart + 1, flagsEnd - flagsStart);

            empathDebug("FLAGS: " + flags);
        }
    }

    int sizePos = line.find("RFC822.SIZE");

    if (-1 == sizePos) {
        empathDebug("No RFC822.SIZE section in string. Weird.");
        return 0;
    } else {

        int sizeStart = line.find(' ', sizePos + 1);
        int sizeEnd = line.find(' ', sizeStart + 1);

        if (
                (-1 == sizeStart) ||
                (-1 == sizeEnd) ||
                (sizeStart >= sizeEnd)
           )
        {
            empathDebug("Couldn't find size");
            return 0;

        } else {

            QString sizeString =
                line.mid(sizeStart + 1, sizeEnd - sizeStart);

            empathDebug("SIZE: " + sizeString);

            size = sizeString.toULong();
        }
    }

    int uidPos = line.find("UID");

    QString id;

    if (-1 == uidPos) {
        empathDebug("No UID section in string. Weird.");
        return 0;
    } else {

        int uidStart = line.find(' ', uidPos + 1);
        int uidEnd = line.find(' ', uidStart + 1);

        if (
                (-1 == uidStart) ||
                (-1 == uidEnd) ||
                (uidStart >= uidEnd)
           )
        {
            empathDebug("Couldn't find uid");
            return 0;

        } else {

            id = line.mid(uidStart + 1, uidEnd - uidStart);
            empathDebug("UID: " + id);

        }
    }

    QCString textForEnvelope;

    QStringList::ConstIterator it(l.begin());

    ++it;

    for (; it != l.end() && (*it)[0] != ')'; ++it)
        textForEnvelope.append((*it).ascii());

    empathDebug("Envelope data:");
    empathDebug(textForEnvelope);
    empathDebug("End of envelope data");

    RMM::Envelope e(textForEnvelope);

    subject = e.subject().asString();
    senderName = e.from().at(0).phrase();
    senderAddress = e.from().at(0).route();
    date = e.date().qdt();
    messageID = e.messageID().asString();

    EmpathIndexRecord * rec =
        new EmpathIndexRecord(
                id,
                subject,
                senderName,
                senderAddress,
                date,
                timeZone,
                status,
                size,
                messageID,
                parentID,
                hasAttachments
         );

    return rec;
}

// vim:ts=4:sw=4:tw=78
