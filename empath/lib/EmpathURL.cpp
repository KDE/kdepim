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
#include <qregexp.h>

// KDE includes
#include <klocale.h>

// Local includes
#include "EmpathURL.h"
#include <rmm/Token.h>

EmpathURL::EmpathURL()
    :   mailboxName_    (QString::null),
        folderPath_     (QString::null),
        messageID_      (QString::null),
        strRep_         (QString::null),
        isValid_        (false)
{
}

EmpathURL::EmpathURL(
        const QString & mailboxName,
        const QString & folderPath,
        const QString & messageID)
    :
        mailboxName_    (mailboxName),
        strRep_         (QString::null),
        isValid_        (true)
{
    folderPath_ = _stripSlashes(folderPath);
    messageID_  = _stripSlashes(messageID);

    _assemble();
}

EmpathURL::EmpathURL(const QString & fullPath)
    :   strRep_(fullPath),
        isValid_(false)
{
    _parse();
    _assemble();
}


EmpathURL::EmpathURL(const EmpathURL & url)
    :   mailboxName_    (url.mailboxName_),
        folderPath_     (url.folderPath_),
        messageID_      (url.messageID_),
        isValid_        (url.isValid_)
{
    _assemble();
}

    void
EmpathURL::_parse()
{
    if (strRep_.left(9) != "empath://") {
        isValid_ = false;
        return;
    }

    isValid_ = true;
    strRep_.remove(0, 9);
    bool hadTrailingSlash(strRep_.at(strRep_.length() - 1) == '/');
    _cleanUp(strRep_);

    if (strRep_.contains('/') == 0) {
        // empath://mailbox_name
        mailboxName_    = strRep_;
        folderPath_     = QString::null;
        messageID_      = QString::null;
        return;
    }

    unsigned int endOfMailboxName  = strRep_.find('/');
    mailboxName_ = strRep_.left(endOfMailboxName);

    if (hadTrailingSlash) {
        // empath://mailbox-name/folder/path/
        folderPath_ = strRep_.mid(endOfMailboxName + 1);
        messageID_  = QString::null;
        return;
    }

    // empath://mailbox-name/folder/path/message-id
    unsigned int endOfFolderPath  = strRep_.findRev('/');

    folderPath_ = strRep_.mid(
        endOfMailboxName + 1, endOfFolderPath - endOfMailboxName - 1);

    messageID_ = strRep_.mid(endOfFolderPath + 1);
}

EmpathURL::~EmpathURL()
{
}

    EmpathURL &
EmpathURL::operator = (const EmpathURL & url)
{
    mailboxName_    = url.mailboxName_;
    folderPath_     = url.folderPath_;
    messageID_      = url.messageID_;
    _assemble();
    return *this;
}

    EmpathURL &
EmpathURL::operator = (const QString & url)
{
    strRep_ = url;
    _parse();
    _assemble();
    return *this;
}

    bool
EmpathURL::operator == (const EmpathURL & b) const
{
    return (
            mailboxName_    == b.mailboxName_   &&
            folderPath_     == b.folderPath_    &&
            messageID_      == b.messageID_);
}

    bool
EmpathURL::operator == (const QString & s) const
{
    EmpathURL url(s);

    return (*this == url);
}


    void
EmpathURL::setMailboxName(const QString & mailboxName)
{
    mailboxName_ = mailboxName;
    _assemble();
}

    void
EmpathURL::setFolderPath(const QString & folderPath)
{
    folderPath_ = folderPath;
    _assemble();
}

    void
EmpathURL::setMessageID(const QString & messageID)
{
    messageID_ = messageID;
    _assemble();
}

    void
EmpathURL::_assemble()
{
    QString s = mailboxName_ + "/" + folderPath_ + "/" + messageID_;
    s.replace(QRegExp("//"), "/");
    strRep_ = "empath://" + s;
}

    QStringList
EmpathURL::folderPathList()
{
    return QStringList::split('/', folderPath_);
}

    EmpathURL
EmpathURL::withoutMessageID() const
{
    EmpathURL url(mailboxName_, folderPath_, QString::null);
    return url;
}

    void
EmpathURL::_cleanUp(QString & s)
{
    while (s.find("//") != -1)
    s.replace(QRegExp("//"), "/");
    if (s.at(0) == '/') s.remove(0, 1);
    if (s.at(s.length() - 1) == '/') s.remove(s.length() - 1, 1);
}

    QString
EmpathURL::mailboxName() const
{
    return _stripSlashes(mailboxName_);
}

    QString
EmpathURL::folderPath() const
{
    return _stripSlashes(folderPath_);
}

    QString
EmpathURL::messageID() const
{
    return _stripSlashes(messageID_);
}

    QString
EmpathURL::_stripSlashes(const QString & s) const
{
    QString stripped(s);
    while (stripped.at(0) == '/')
        stripped.remove(0, 1);

    while (stripped.at(s.length()) == '/')
        stripped.remove(s.length() - 1, 1);

    return stripped;
}

// vim:ts=4:sw=4:tw=78
