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
# pragma implementation "EmpathConfigPOP3Logging.h"
#endif

// Qt includes
#include <qpixmap.h>
#include <qlabel.h>
#include <qlayout.h>

// KDE includes
#include <klocale.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathPathSelectWidget.h"
#include "EmpathConfigPOP3Logging.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathUIUtils.h"

EmpathConfigPOP3Logging::EmpathConfigPOP3Logging
    (const EmpathURL & url, QWidget * parent)
    :   QWidget(parent, "ConfigPOP3Logging"),
        url_(url)
{
    cb_logConversation_ =
        new QCheckBox(i18n("Log conversations with the server"),
            this, "cb_logConversation");
    
    QLabel * l_logFile = new QLabel(i18n("Log file"), this, "l_logFile");
    
    efsw_logFile_ = new EmpathFileSelectWidget(QString::null, this);
    
    cb_appendToLog_ = new QCheckBox(
            i18n("Append to log file (rather than overwrite)"),
            this, "cb_appendToLog_");

    pb_viewCurrentLog_ = new QPushButton(
            i18n("View log file"), this, "pb_viewCurrentLog");
    
    QLabel * l_maxLogFileSize = new QLabel(
            i18n("Maximum size of log file"), this, "l_maxLogFileSize");
    
    sb_maxLogFileSize_ =
        new QSpinBox(0, 10000, 10, this, "sb_maxLogFileSize");
    
    QLabel * l_logFileKb  = new QLabel("Kb", this, "l_logFileKb");
    
    // Layout
    
    QVBoxLayout * topLevelLayout = new QVBoxLayout(this, 10, 10);

    QHBoxLayout * layout0 = new QHBoxLayout(topLevelLayout);
    layout0->addWidget(cb_logConversation_);
    layout0->addWidget(pb_viewCurrentLog_);
    
    topLevelLayout->addWidget(cb_appendToLog_);
    
    QHBoxLayout * layout1 = new QHBoxLayout(topLevelLayout);
    layout1->addWidget(l_logFile);
    layout1->addWidget(efsw_logFile_);

    QHBoxLayout * layout2 = new QHBoxLayout(topLevelLayout);
    layout2->addWidget(l_maxLogFileSize);
    layout2->addWidget(sb_maxLogFileSize_);
    layout2->addWidget(l_logFileKb);
}

EmpathConfigPOP3Logging::~EmpathConfigPOP3Logging()
{
    // Empty.
}

    void
EmpathConfigPOP3Logging::saveData()
{
    EmpathMailbox * mailbox = empath->mailbox(url_);

    if (mailbox == 0)
        return;

    if (mailbox->type() != EmpathMailbox::POP3) {
        empathDebug("Incorrect mailbox type");
        return;
    }

    EmpathMailboxPOP3 * m = (EmpathMailboxPOP3 *)mailbox;

    m->setLoggingPolicy         (cb_logConversation_->isChecked());
    m->setLogFilePath           (efsw_logFile_->selected());
    m->setLogFileDisposalPolicy (cb_appendToLog_->isChecked());
    m->setMaxLogFileSize        (sb_maxLogFileSize_->value());
}

    void
EmpathConfigPOP3Logging::loadData()
{
    EmpathMailbox * mailbox = empath->mailbox(url_);

    if (mailbox == 0)
        return;

    if (mailbox->type() != EmpathMailbox::POP3) {
        empathDebug("Incorrect mailbox type");
        return;
    }

    EmpathMailboxPOP3 * m = (EmpathMailboxPOP3 *)mailbox;

    cb_logConversation_ ->setChecked    (m->loggingPolicy());
    efsw_logFile_       ->setPath       (m->logFilePath());
    cb_appendToLog_     ->setChecked    (m->logFileDisposalPolicy());
    sb_maxLogFileSize_  ->setValue      (m->maxLogFileSize());
}

    void
EmpathConfigPOP3Logging::s_viewLog()
{
}

// vim:ts=4:sw=4:tw=78
