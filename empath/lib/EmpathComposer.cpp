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
# pragma implementation "EmpathComposer.h"
#endif

//Qt includes
#include <qfile.h>
#include <qtextstream.h>

// KDE includes
#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

// Local includes
#include "EmpathComposer.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathQuotedText.h"
#include <RMM_DateTime.h>
#include <RMM_Token.h>
#include <RMM_AddressList.h>

EmpathComposer::EmpathComposer()
    :   QObject()
{
    empathDebug("ctor");
    QObject::connect(
        empath, SIGNAL(retrieveComplete(bool, const EmpathURL &, QString)),
        SLOT(s_retrieveComplete(bool, const EmpathURL &, QString)));
}

EmpathComposer::~EmpathComposer()
{
    empathDebug("dtor");
}

     RMM::RMessage 
EmpathComposer::message(EmpathComposer::Form & composeForm)
{
    empathDebug("message() called");

    RMM::RMessage message;
    
    // Copy the visible headers first.
    RMM::REnvelope envelope = composeForm.visibleHeaders;

    // Now add the invisible headers. Should really implement
    // REnvelope::operator +, but for now this will do.
    RMM::RHeaderList invisibleHeaders =
        composeForm.invisibleHeaders.headerList();

    RMM::RHeaderListIterator it(invisibleHeaders);

    for (; it.current(); ++it)
        envelope.addHeader(*it.current());

    message.setEnvelope(envelope);

    // Now we need to know what type the main body part has.
    // We really need to find a nice way to send 16-bit Unicode.
    //
    // If the text is UTF-8 (8-bit) we can elect either to send
    // it encoded as 8-bit (i.e., don't do anything) or we could
    // encode it as quoted-printable.
    //
    // Either of these will work, but which is the best ? This
    // needs to be thought about hard or decided on experience.
    //
    // My gut feeling is that it's better to simply send as 8-bit.
    // This is not because I'm lazy, more because I've seen more
    // MUAs that don't decode qp than MTAs that can't handle 8-bit.
    //
    // The problem is that while many sendmail installations don't
    // advertise that they handle 8-bit, they really do. I'm not
    // going to spend my life worrying about broken sendmail.

    message.setData(composeForm.body + "\n-- \n" + _signature());

    QValueList<EmpathAttachmentSpec>::Iterator it2 =
        composeForm.attachments.begin();
   
    for (; it2 != composeForm.attachments.end(); it2++) {

        RMM::RBodyPart * newPart = new RMM::RBodyPart;
        newPart->setDescription((*it2).description().utf8());
        newPart->setEncoding((*it2).encoding());
        newPart->setMimeType((*it2).type().utf8());
        newPart->setMimeSubType((*it2).subType().utf8());
// TODO        newPart.setCharset((*it2).charset());

        message.addPart(newPart);
    }

    return message;
}

    void
EmpathComposer::newComposeForm(const QString & recipient)
{
    EmpathComposer::Form composeForm;
    composeForm.composeType = ComposeNormal;
    composeForm.invisibleHeaders.set("To", recipient.local8Bit());
    _initVisibleHeaders(composeForm);
    emit(composeFormComplete(composeForm));
}

    void
EmpathComposer::newComposeForm(   
    EmpathComposer::ComposeType t, const EmpathURL & url)
{
    int id = 0;
    EmpathComposer::Form composeForm;

    composeForm.composeType = t;
    
    // find an unused id, if everything is allright this shouldn't take too long.
    while (jobs_.contains(id))
        id++;
    
    jobs_.insert(id, composeForm); 
    
    empath->retrieve(url, "Composer: " + QString::number(id));
}

    void
EmpathComposer::bugReport()
{
    EmpathComposer::Form composeForm;

    composeForm.composeType = ComposeNormal;
    composeForm.invisibleHeaders.set("To", 
        ( EMPATH_MAINTAINER + " <" + EMPATH_MAINTAINER_EMAIL +">").local8Bit() ); 
    // Note: Leave ' ' at start of lines > 0 to conform to RFC822 header spec.
    composeForm.invisibleHeaders.set("X-EmpathInfo",
        "Empath Version: "  + EMPATH_VERSION_STRING.local8Bit() + "\n"
        " KDE Version: "    + KDE_VERSION_STRING + "\n"
        " Qt Version: "     + QT_VERSION_STR + "\n");

    composeForm.body = 
    "- " +
    i18n("What were you trying to do when the problem occured ?") +
    "\n\n\n\n" +
    "- " +
    i18n("What actually happened ?") +
    "\n\n\n\n" +
    "- " +
    i18n("Exactly what did you do that caused the problem to manifest itself ?") +
    "\n\n\n\n" +
    "- " +
    i18n("Do you have a suggestion as to how this behaviour can be corrected ?") +
    "\n\n\n\n" +
    "- " +
    i18n("If you saw an error message, please try to reproduce it here.");

    _initVisibleHeaders(composeForm);
    emit(composeFormComplete(composeForm));
}
 
    void
EmpathComposer::s_retrieveComplete(
        bool status, const EmpathURL & url, QString xinfo)
{
    if (!status || strcmp(xinfo.left(8), "Composer") != 0)
        return;

    RMM::RMessage * m(empath->message(url, xinfo));
    
    if (m == 0) {
        empathDebug(
            "Couldn't get supposedly retrieved message `" +
            url.asString() + "'");
        return;
    }
 
    int id = xinfo.remove(0, 10).toInt();

    switch (jobs_[id].composeType) {

        case EmpathComposer::ComposeReply:      
            _reply(id, m);       
            break; 
        case EmpathComposer::ComposeReplyAll:   
            _reply(id, m);
            break;
        case EmpathComposer::ComposeForward:    
            _forward(id, m);
            break; 
        case EmpathComposer::ComposeBounce:
            _bounce(id, m);
            break;
        case EmpathComposer::ComposeNormal:      
        default:                                        
            break;
    }

    EmpathComposer::Form composeForm(jobs_[id]);
    jobs_.remove(id);
    _initVisibleHeaders(composeForm);
    emit(composeFormComplete(composeForm));
}

    void
EmpathComposer::_initVisibleHeaders(EmpathComposer::Form & composeForm)
{ 
    KConfig * c(KGlobal::config());
    c->setGroup(EmpathConfig::GROUP_COMPOSE);
    
    QStrList l;
    c->readListEntry(EmpathConfig::C_EXTRA_HEADERS, l, ',');
    empathDebug("There are " + QString().setNum(l.count()) + " headers");

    // Standard headers
    l.prepend("Bcc");
    l.prepend("Cc");
    l.prepend("To");
    l.append("Subject");

    QStrListIterator it(l);
    for (; it.current(); ++it)
        if (composeForm.invisibleHeaders.has(it.current()))
            composeForm.visibleHeaders.addHeader(
                composeForm.invisibleHeaders.get(it.current())->asString());
        else
            composeForm.visibleHeaders.addHeader(QCString(it.current()) + QCString(":"));
}

   void
EmpathComposer::_reply(int id, RMM::RMessage * m)
{
    empathDebug("Replying");
 
    EmpathComposer::Form composeForm(jobs_[id]);
    RMM::RMessage message(*m);
    QCString to, cc;
    KConfig * c(KGlobal::config());
    
    referenceHeaders_ = _referenceHeaders(m);
    
    // First fill in the primary return address. This will be the Reply-To
    // address if there's one given, otherwise it will be the first in
    // the sender list.

    if (message.envelope().has(RMM::HeaderReplyTo)) 
        to = message.envelope().replyTo().at(0)->asString();
    else if (message.envelope().has(RMM::HeaderFrom)) 
        to = message.envelope().from().at(0)->asString();
    // Warn user if no 'to' can be found?

    composeForm.invisibleHeaders.set("To", to);

    if (composeForm.composeType == EmpathComposer::ComposeReplyAll) {
        
        if (message.envelope().has(RMM::HeaderCc)) 

            for (uint i = 0; i < message.envelope().cc().count(); i++) {
                if (i > 0)
                    cc += ", ";
                cc += message.envelope().cc().at(i)->asString();
            }
    
        c->setGroup("UserInfo");
        
        RMM::RAddress me(c->readEntry("EmailAddress").ascii());
        
        if (!(me == *message.envelope().to().at(0)))
            if (!cc.isEmpty()) 
                cc += message.envelope().to().asString();
        
        composeForm.invisibleHeaders.set("Cc", cc);
    }
    
    // Fill in the subject.
    QCString s = message.envelope().subject().asString();
    empathDebug("Subject was \"" + s + "\""); 

    if (s.isEmpty())
        composeForm.invisibleHeaders.set(
                "Subject", i18n("Re: (no subject given)").local8Bit());
    else
        if (s.find(QRegExp("^[Rr][Ee]:")) != -1)
            composeForm.invisibleHeaders.set("Subject", s);
        else
            composeForm.invisibleHeaders.set("Subject", "Re: " + s);

    // Now quote original message if we need to.
    
    c->setGroup(EmpathConfig::GROUP_COMPOSE);
    
    empathDebug("Quoting original if necessary");

    // Add the 'On (date) (name) wrote' bit
        
    if (c->readBoolEntry(EmpathConfig::C_AUTO_QUOTE)) {

        s = message.data();

        // Remove the signature
        int sigpos = s.find("\n-- \n");
        if (sigpos != -1)
            s.truncate(sigpos);
        
        _quote(s); 
        
        QString thingyWrote = c->readEntry(
            composeForm.composeType == EmpathComposer::ComposeReplyAll
                    ? EmpathConfig::C_PHRASE_REPLY_ALL
                    : EmpathConfig::C_PHRASE_REPLY_SENDER 
            , "");
        
        // Be careful here. We don't want to reveal people's
        // email addresses.
        if (message.envelope().has(RMM::HeaderFrom) &&
            !message.envelope().from().at(0)->phrase().isEmpty()) {
            
            thingyWrote.replace(QRegExp("\\%s"),
                message.envelope().from().at(0)->phrase());

            if (message.envelope().has(RMM::HeaderDate))
                thingyWrote.replace(QRegExp("\\%d"),
                    message.envelope().date().qdt().date().toString());
            else
                thingyWrote.replace(QRegExp("\\%d"),
                    i18n("An unknown date and time"));
        
            composeForm.body = '\n' + thingyWrote.local8Bit() + '\n' + s;
        }
    }

    jobs_.replace(id, composeForm);
}
    
    void
EmpathComposer::_forward(int id, RMM::RMessage * m)
{
    empathDebug("Forwarding");
    
    EmpathComposer::Form composeForm(jobs_[id]);
    RMM::RMessage message(*m);
    QCString s;

    // Fill in the subject.
    s = message.envelope().subject().asString();
    empathDebug("Subject was \"" + s + "\""); 

    if (s.isEmpty()) 
        composeForm.invisibleHeaders.set(
                "Subject", i18n("Fwd: (no subject given)").local8Bit());
    else
        if (s.find(QRegExp("^[Ff][Ww][Dd]:")) != -1)
            composeForm.invisibleHeaders.set("Subject", s);
        else
            composeForm.invisibleHeaders.set("Subject", "Fwd: " + s);

    if (message.body().count() == 0)
        composeForm.body = message.decode().data();
    else if (message.body().count() == 1)
        composeForm.body = message.body().at(0)->decode().data();
    else {
        // TODO
    } 

    jobs_.replace(id, composeForm);
}

    void
EmpathComposer::_bounce(int, RMM::RMessage * /* m */)
{
    // TODO
}

    QCString
EmpathComposer::_referenceHeaders(RMM::RMessage * m)
{
    QCString s;

    RMM::RMessage message(*m);
    
    // Ok, here's the system.
    // Whatever happens, we create an In-Reply-To.
    
    s += "In-Reply-To: ";
    s += message.envelope().messageID().asString();
    s += "\n";
    
    if (!message.envelope().has(RMM::HeaderReferences)) {
        
        s += "References: ";
        s += message.envelope().messageID().asString();
        s += "\n";
        
        return s;
    }
    
    // There is a references header.

    QCString references = "References: ";
    
    QCString refs(message.envelope().references().asString());
    
    QStrList l;
    
    RMM::RTokenise(refs, " ", l);
    
    // While there are more than 10 references, remove the one in
    // the second position. This way, we'll end up with 10 again,
    // and remove all the older references, excluding the oldest.
        
    while (l.count() >= 10)
        l.remove(2);
    
    QStrListIterator it(l);

    for (; it.current(); ++it) {
        references += it.current();
        references += " ";
    }
    
    references += message.envelope().messageID().asString();
    s += references;
    s += "\n";
    
    return s;
}

    QCString
EmpathComposer::_stdHeaders()
{
    QCString s;
    
    KConfig * c(KGlobal::config());
    c->setGroup("UserInfo");

    s += QCString("From: ");
    s += QCString(c->readEntry("FullName").ascii());
    s += QCString(" <");
    s += QCString(c->readEntry("EmailAddress").ascii());
    s += QCString(">");
    s += "\n";
    
    RMM::RDateTime dt;
    dt.createDefault();
    s += "Date: " + dt.asString();
    s += "\n";
    
    RMM::RMessageID id;
    id.createDefault();
    
    s += "Message-Id: " + id.asString();
    s += "\n";

    return s;
}

    QCString
EmpathComposer::_signature()
{
    QCString s;

    KConfig * c(KGlobal::config());

    c->setGroup(EmpathConfig::GROUP_COMPOSE);
    
    if (!c->readBoolEntry(EmpathConfig::C_ADD_SIG))
        return s;

    QFile f(c->readEntry(EmpathConfig::C_SIG_PATH));
    
    if (f.open(IO_ReadOnly)) {    
        QTextStream t(&f);
        s += t.read().ascii();
    }
    
    return s;
}

    void
EmpathComposer::_quote(QCString & s)
{
    EmpathQuotedText quoted(QString::fromLocal8Bit(s));
    quoted.rewrap(70);
    quoted.quote();
    s = quoted.asString().local8Bit();
}

// vim:ts=4:sw=4:tw=78
