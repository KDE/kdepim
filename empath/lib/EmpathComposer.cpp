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
#include "EmpathJobScheduler.h"
#include "EmpathComposer.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathQuotedText.h"
#include <RMM_DateTime.h>
#include <RMM_Token.h>
#include <RMM_AddressList.h>
#include <RMM_Utility.h>

EmpathComposer * EmpathComposer::THIS = 0L;

EmpathComposer::EmpathComposer()
    :   QObject()
{
    QObject::connect(
        this,   SIGNAL(composeFormComplete(EmpathComposeForm)),
        empath, SIGNAL(newComposer(EmpathComposeForm)));
}

EmpathComposer::~EmpathComposer()
{
}

     RMM::RMessage 
EmpathComposer::message(EmpathComposeForm composeForm)
{
    RMM::RMessage message;
    
    // Copy the visible headers first.
    RMM::REnvelope envelope = composeForm.visibleHeaders();

    // Now add the invisible headers. Should really implement
    // REnvelope::operator +, but for now this will do.
    RMM::RHeaderList invisibleHeaders =
        composeForm.invisibleHeaders().headerList();

    for (RMM::RHeaderListIterator it(invisibleHeaders); it.current(); ++it)
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

    QCString body = composeForm.body();

    body += _signature();

    message.setData(body);

    EmpathAttachmentSpecList attachments = composeForm.attachmentList();

    QValueList<EmpathAttachmentSpec>::Iterator it2 = attachments.begin();
   
    for (; it2 != attachments.end(); it2++) {

        RMM::RBodyPart * newPart = new RMM::RBodyPart;

        newPart->setDescription ((*it2).description().utf8());
        newPart->setEncoding    ((*it2).encoding());
        newPart->setMimeType    ((*it2).type().utf8());
        newPart->setMimeSubType ((*it2).subType().utf8());
        newPart->setCharset     ((*it2).charset().utf8());

        message.addPart(newPart);
    }

    return message;
}

    void
EmpathComposer::newComposeForm(const QString & recipient)
{
    EmpathComposeForm composeForm;
    composeForm.setComposeType(ComposeNormal);
    _initVisibleHeaders(composeForm);
    composeForm.setHeader("To", recipient.utf8(), false);
    emit(composeFormComplete(composeForm));
}

    void
EmpathComposer::newComposeForm(ComposeType t, const EmpathURL & url)
{
    EmpathComposeForm composeForm;

    composeForm.setComposeType(t);
    
    EmpathJobID id = empath->retrieve(url, this);

    empathDebug("Created a job with id " + QString::number(id));

    jobList_.insert(id, composeForm); 
}

    void
EmpathComposer::s_retrieveJobFinished(EmpathRetrieveJob j)
{
    if (!(jobList_.contains(j.id()))) {
        empathDebug("jobList does not contain a job with id " + QString::number(j.id()));
        return;
    }

    if (!j.success()) {
        empathDebug("job was unsuccessful");
        return;
    }

    RMM::RMessage m(j.message());
    
    switch (jobList_[j.id()].composeType()) {

        case ComposeReply:              _reply(j.id(), m);      break; 
        case ComposeReplyAll:           _reply(j.id(), m);      break;
        case ComposeForward:            _forward(j.id(), m);    break; 
        case ComposeBounce:             _bounce(j.id(), m);     break;
        case ComposeNormal: default:                            break;
    }

    EmpathComposeForm composeForm(jobList_[j.id()]);
    jobList_.remove(j.id());
    _initVisibleHeaders(composeForm);
    emit(composeFormComplete(composeForm));
}

    void
EmpathComposer::_initVisibleHeaders(EmpathComposeForm & composeForm)
{ 
    KConfig * config(KGlobal::config());
    config->setGroup(EmpathConfig::GROUP_COMPOSE);
    
    QStrList l;

    config->readListEntry(EmpathConfig::C_EXTRA_HEADERS, l, ',');

    // Standard headers
    l.prepend("Bcc");
    l.prepend("Cc");
    l.prepend("To");
    l.append("Subject");

    for (QStrListIterator it(l); it.current(); ++it)
        composeForm.setHeader(it.current(), "");
}

   void
EmpathComposer::_reply(EmpathJobID id, RMM::RMessage message)
{
    empathDebug("");
 
    EmpathComposeForm composeForm(jobList_[id]);
    QCString to, cc;
    KConfig * config = new KConfig("emaildefaults");
    
    referenceHeaders_ = _referenceHeaders(message);
    
    // First fill in the primary return address. This will be the Reply-To
    // address if there's one given, otherwise it will be the first in
    // the sender list.

    if (message.envelope().has(RMM::HeaderReplyTo)) 
        to = message.envelope().replyTo().at(0).asString();
    else if (message.envelope().has(RMM::HeaderFrom)) 
        to = message.envelope().from().at(0).asString();
    else
        to = i18n("Could not find sender of this message").ascii() +
            QCString(" <postmaster@localhost>");

    composeForm.setHeader("To", to);

    if (composeForm.composeType() == ComposeReplyAll) {
        
        if (message.envelope().has(RMM::HeaderCc)) 

            for (unsigned int i = 0; i < message.envelope().cc().count(); i++) {
                if (i > 0)
                    cc += ", ";
                if (cc.length() > 70)
                    cc += "\r\n ";
                cc += message.envelope().cc().at(i).asString();
            }
    
        config->setGroup("UserInfo");
        
        RMM::RAddress me(config->readEntry("EmailAddress").ascii());
        RMM::RAddress msgTo(message.envelope().to().at(0));
        
        if (me != msgTo && !cc.isEmpty()) 
            cc += message.envelope().to().asString();
        
        composeForm.setHeader("Cc", cc);
    }
    
    // Fill in the subject.
    QCString s = message.envelope().subject().asString();
    empathDebug("Subject was \"" + s + "\""); 

    if (s.isEmpty())
        composeForm.setHeader(
                "Subject", i18n("Re: (no subject given)").utf8());
    else
        if (s.find(QRegExp("^[Rr][Ee]:")) != -1)
            composeForm.setHeader("Subject", s);
        else
            composeForm.setHeader("Subject", "Re: " + s);

    // Now quote original message if we need to.
    
    config->setGroup(EmpathConfig::GROUP_COMPOSE);
    
    // Add the 'On (date) (name) wrote' bit
        
    if (config->readBoolEntry(EmpathConfig::C_AUTO_QUOTE, false)) {

        empathDebug("Quoting original");

        s = message.data();

        // Remove the signature
        int sigpos = s.find(QRegExp("\n--[ ]?\n"));
        if (sigpos != -1)
            s.truncate(sigpos);
        
        _quote(s); 
        
        QString thingyWrote;
       
        if (composeForm.composeType() == ComposeReplyAll)
            thingyWrote =
                config->readEntry(EmpathConfig::C_PHRASE_REPLY_ALL, "");
        else
            thingyWrote =
                config->readEntry(EmpathConfig::C_PHRASE_REPLY_SENDER, "");
        
        // Be careful here. We don't want to reveal people's
        // email addresses.
        if (message.envelope().has(RMM::HeaderFrom) &&
            !message.envelope().from().at(0).phrase().isEmpty()) {
            
            thingyWrote.replace(QRegExp("\\%s"),
                message.envelope().from().at(0).phrase());

            if (message.envelope().has(RMM::HeaderDate))
                thingyWrote.replace(QRegExp("\\%d"),
                    message.envelope().date().qdt().date().toString());
            else
                thingyWrote.replace(QRegExp("\\%d"),
                    i18n("An unknown date and time"));
        
            composeForm.setBody('\n' + thingyWrote.utf8() + '\n' + s);
        }
    }

    jobList_.replace(id, composeForm);

    delete config;
}
    
    void
EmpathComposer::_forward(EmpathJobID id, RMM::RMessage message)
{
    empathDebug("");
    
    EmpathComposeForm composeForm(jobList_[id]);
    QCString s;

    // Fill in the subject.
    s = message.envelope().subject().asString();
    empathDebug("Subject was \"" + s + "\""); 

    if (s.isEmpty()) 
        composeForm.setHeader("Subject", i18n("Fwd: (no subject)").utf8());
    else
        if (s.find(QRegExp("^[Ff][Ww][Dd]:")) != -1)
            composeForm.setHeader("Subject", s);
        else
            composeForm.setHeader("Subject", "Fwd: " + s);

    QString description = i18n("Forwarded message from %1");

    QString descriptionFilled =
        description.arg(message.envelope().firstSender().asString());

    message.setDescription(description.utf8());

    // TODO: Add setPreamble() to RBodyPart
//    message.setPreamble(descriptionFilled.utf8())

    // TODO: Implement setDisposition() in RBodyPart
//    message.setDisposition(RMM::DispositionTypeInline);

    // TODO: Add setDispositionFilename() to RBodyPart
//    message.setDispositionFilename("message-" +
//        message.envelope().messageID().asString());

    message.setMimeType(RMM::MimeTypeMessage);
    message.setMimeSubType(RMM::MimeSubTypeRFC822);

    // Forward encoded as Base64. Wise ? XXX
    message.setEncoding(RMM::CteTypeBase64);

    composeForm.setBody(RMM::encodeBase64(message.asString()));

    jobList_.replace(id, composeForm);
}

    void
EmpathComposer::_bounce(EmpathJobID, RMM::RMessage /* m */)
{
    // TODO
}

    QCString
EmpathComposer::_referenceHeaders(RMM::RMessage message)
{
    QCString s;

    // Ok, here's the system.
    // Whatever happens, we create an In-Reply-To.
    
    s += "In-Reply-To: " + message.envelope().messageID().asString() + "\n";
    
    if (!message.envelope().has(RMM::HeaderReferences)) {
        s += "References: " + message.envelope().messageID().asString() + "\n";
        return s;
    }
    
    // There is a references header.

    QCString references;
    
    QCString refs(message.envelope().references().asString());
    
    QStrList l;
    
    RMM::RTokenise(refs, " ", l);
    
    // While there are more than 10 references, remove the one in
    // the second position. This way, we'll end up with 10 again,
    // and remove all the older references, excluding the oldest.
        
    while (l.count() > 9)
        l.remove(2);
    
    QStrListIterator it(l);

    for (; it.current(); ++it) {
        references += it.current();
        references += " ";
    }
    
    references += message.envelope().messageID().asString();

    s += "References: " + references + "\n";
    
    return s;
}

    QCString
EmpathComposer::_stdHeaders()
{
    QCString s;
    
    KConfig * config = new KConfig("emaildefaults");
    config->setGroup("UserInfo");

    QCString fullName = config->readEntry("FullName").ascii();
    QCString emailAddress = config->readEntry("EmailAddress").ascii();
    QCString organization = config->readEntry("Organization").ascii();

    if (fullName.isEmpty()) {
        empathDebug("Name of user is empty.");
    }

    if (emailAddress.isEmpty()) {

        // Shit !
        empathDebug("User's email address is empty !");
        emailAddress = "no.address.given@nowhere";
    }

    s += "From: " + fullName + " <" + emailAddress + ">\n";
 
    if (!organization.isEmpty())
        s += "Organization: " + organization + "\n";
    
    RMM::RDateTime dt;
    dt.createDefault();
    s += "Date: " + dt.asString() + "\n";
    
    RMM::RMessageID id;
    id.createDefault();
    
    s += "Message-Id: " + id.asString() + "\n";

    delete config;

    return s;
}

    QCString
EmpathComposer::_signature()
{
    QCString sig;

    KConfig * config(KGlobal::config());

    using namespace EmpathConfig;

    config->setGroup(GROUP_COMPOSE);
    
    if (config->readBoolEntry(C_ADD_SIG, false)) {

        QFile f(config->readEntry(C_SIG_PATH));
        
        if (f.open(IO_ReadOnly)) {    
            QTextStream t(&f);
            QCString s;
            t >> s;
            sig = "\n-- \n" + s;
        }
    }
    
    return sig;
}

    void
EmpathComposer::_quote(QCString & s)
{
    empathDebug(s);
    EmpathQuotedText quoted(QString::fromUtf8(s));
    quoted.rewrap(70);
    quoted.quote();
    s = quoted.asString().utf8();
    empathDebug(s);
}

// vim:ts=4:sw=4:tw=78
