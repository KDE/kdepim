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
# pragma implementation "EmpathMessageDownloadStatus.h"
#endif

// KDE includes
#include <klocale.h>

// Local includes
#include "EmpathMessageDownloadStatus.h"

EmpathMessageDownloadStatus::EmpathMessageDownloadStatus(
        QWidget * parent,
        const char * name)
    :    QWidget(parent, name),
        numMsgs_(0),
        totalMailboxBytes_(0),
        msgNo_(0)
{
    empathDebug("ctor");

    mainLayout_ = new QHBoxLayout(this, 0, 10);
    
    l_messageNumber_    =
        new QLabel(this, "l_messageNumber");
    CHECK_PTR(l_messageNumber_);

    l_totalKbLeft_        =
        new QLabel(this, "l_totalKbLeft");
    CHECK_PTR(l_totalKbLeft_);

    pr_allMessages_        =
        new KProgress(0, 100, 0, KProgress::Horizontal, this, "pr_allMessages");
    CHECK_PTR(pr_allMessages_);

    l_messageNumber_->setText(
            i18n("Message") +
            QString(" 9999/9999"));
    
    l_messageNumber_->setMinimumWidth(l_messageNumber_->sizeHint().width());
    
    l_totalKbLeft_->setText(
            i18n("Total") +
            QString(" ......Kb ")
            + i18n("left"));
    
    l_totalKbLeft_->setMinimumWidth(l_totalKbLeft_->sizeHint().width());
    
    pr_allMessages_->setFixedWidth(60);

    mainLayout_->addWidget(l_messageNumber_);
    mainLayout_->addWidget(pr_allMessages_);
    mainLayout_->addWidget(l_totalKbLeft_);
    mainLayout_->activate();
    
}

EmpathMessageDownloadStatus::~EmpathMessageDownloadStatus()
{
    empathDebug("dtor");
}

    void
EmpathMessageDownloadStatus::s_setNumberOfMessages(Q_UINT32 numMessages)
{
    msgNo_        = 0;
    numMsgs_    = numMessages;
    l_messageNumber_->setText(i18n("Message 0/") + QString().setNum(numMsgs_));
    empathDebug("Message number set to " + QString(l_messageNumber_->text()));
}

    void
EmpathMessageDownloadStatus::s_setInitialMailboxBytes(Q_UINT32 mailboxBytes)
{
    initialTotalMailboxBytes_ = mailboxBytes;
}

    void
EmpathMessageDownloadStatus::s_setMailboxBytes(Q_UINT32 mailboxBytes)
{
    totalMailboxBytes_ = mailboxBytes;
    l_totalKbLeft_->setText(
            i18n("Total ") +
            QString().setNum(totalMailboxBytes_ / 1024) +
            i18n("Kb left"));
    empathDebug("Total Kb left set to " + QString(l_totalKbLeft_->text()));
    pr_allMessages_->setValue(
            100 -
            (totalMailboxBytes_ / (float)initialTotalMailboxBytes_) * 100);
}

    void
EmpathMessageDownloadStatus::s_nextMessage(Q_UINT32)
{
    ++msgNo_;
    l_messageNumber_->setText(
            i18n("Message") +
            QString(" ") + 
            QString().setNum(msgNo_) +
            "/" +
            QString().setNum(numMsgs_));
    empathDebug("Message No. set to " + QString(l_messageNumber_->text()));
}

// vim:ts=4:sw=4:tw=78
