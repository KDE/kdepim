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


// KDE includes
#include <kglobal.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "rmm/Message.h"
#include "rmm/Address.h"
#include "rmm/Envelope.h"
#include "EmpathMailSenderSMTP.h"
#include "Empath.h"

EmpathMailSenderSMTP::EmpathMailSenderSMTP()
    :   EmpathMailSenderImpl()
{
//    job_ = new KIO::Job;
}

EmpathMailSenderSMTP::~EmpathMailSenderSMTP()
{
    delete job_;
    job_ = 0;
}

    void
EmpathMailSenderSMTP::setServer(const QString & _name, const Q_UINT32 _port)
{
    serverName_ = _name;
    serverPort_ = _port;
}

   void
EmpathMailSenderSMTP::sendOne(RMM::Message m, const QString & id)
{
    currentID_ = id;

    KConfig c(KGlobal::dirs()->findResource("config", "kcmemailrc"), true);

    QString sender = c.readEntry("EmailAddress");

    QString recipient;

    RMM::AddressList addressList = m.envelope().to();

    if (addressList.count() == 0) {
        // XXX: We're not sending to anyone ?
        return;
    }

    RMM::Address address = addressList.at(0);

    if (address.type() == RMM::Address::AddressTypeGroup) {
        // FIXME: Handle sending to a group.
        return;
    }

    recipient = address.localPart();
    recipient += '@';
    recipient += address.domain();

    QString putStr = "smtp://" +
        serverName_ + ':' + QString().setNum(serverPort_) +
        '/' +
        sender + ',' + recipient;

//    job_->put(putStr.ascii(), 0, false, false, 0);
}

    void
EmpathMailSenderSMTP::saveConfig()
{
    KConfig * c = KGlobal::config();

    c->setGroup("Sending");
    c->writeEntry("SMTPServerLocation", serverName_);
    c->writeEntry("SMTPServerPort", serverPort_);
}

    void
EmpathMailSenderSMTP::loadConfig()
{
    KConfig * c = KGlobal::config();

    c->setGroup("Sending");
    serverName_ = c->readEntry("SMTPServerLocation", "localhost");
    serverPort_ = c->readUnsignedNumEntry("SMTPServerPort", 25);
}

    void
EmpathMailSenderSMTP::s_jobError(int, int, const char *)
{
//    sendCompleted(currentID_, false);
}

    void
EmpathMailSenderSMTP::s_jobFinished(int)
{
//    sendCompleted(currentID_, true);
}

    void
EmpathMailSenderSMTP::s_jobCanceled(int)
{
//    sendCompleted(currentID_, false);
}

    void
EmpathMailSenderSMTP::s_jobReady(int)
{
//    job_->slotData(0, 0);
}

// vim:ts=4:sw=4:tw=78
