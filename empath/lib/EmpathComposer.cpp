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


//Qt includes
#include <qfile.h>
#include <qtextstream.h>

// KDE includes
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

// Local includes
#include "EmpathJobScheduler.h"
#include "EmpathComposer.h"
#include "Empath.h"
#include "EmpathQuotedText.h"
#include <rmm/DateTime.h>
#include <rmm/Token.h>
#include <rmm/AddressList.h>
#include <rmm/Utilities.h>

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

     RMM::Message 
EmpathComposer::message(EmpathComposeForm composeForm)
{
    RMM::Message message;

    // Copy the visible headers first.
    QMap<QString, QString> envelope = composeForm.visibleHeaders();

    // Now add the invisible headers. Should really implement
    // REnvelope::operator +, but for now this will do.
    QMap<QString, QString> invisibleHeaders = composeForm.invisibleHeaders();

    QMap<QString, QString>::ConstIterator it(invisibleHeaders.begin());

    for (; it != invisibleHeaders.end(); ++it)
        envelope.insert(it.key(), it.data());

// TODO    message.setEnvelope(envelope);

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

    QString body = composeForm.body();

    body += _signature();

    message.setData(body.utf8()); // FIXME: Encode !

    EmpathAttachmentSpecList attachments = composeForm.attachmentList();

    QValueList<EmpathAttachmentSpec>::Iterator it2 = attachments.begin();

    for (; it2 != attachments.end(); it2++) {

        RMM::BodyPart * newPart = new RMM::BodyPart;

        newPart->setDescription ((*it2).description().utf8());
        // FIXME: Need to keep in sync to do this !
        newPart->setEncoding    (RMM::CteType((*it2).encoding()));
        newPart->setMimeGroup   ((*it2).type().utf8());
        newPart->setMimeValue   ((*it2).subType().utf8());
        newPart->setCharset     ((*it2).charset().utf8());

        message.addPart(newPart);
    }

    return message;
}

    void
EmpathComposer::newComposeForm(const QString & recipient)
{
    EmpathComposeForm composeForm;
    _initVisibleHeaders(composeForm);
    composeForm.setHeader(QString::fromUtf8("To"), recipient, false);
    emit(composeFormComplete(composeForm));
}

    void
EmpathComposer::newComposeForm(
    EmpathComposeForm::ComposeType t,
    const EmpathURL & url
)
{
    EmpathComposeForm composeForm;

    composeForm.setComposeType(t);

    EmpathJobID id = empath->retrieve(url, this);

    jobList_.insert(id, composeForm); 
}

    void
EmpathComposer::s_retrieveJobFinished(EmpathRetrieveJob j)
{
    if (!(jobList_.contains(j.id()))) {
        empathDebug(QString::fromUtf8("jobList does not contain a job with id ") + QString::number(j.id()));
        return;
    }

    if (!j.success()) {
        empathDebug(QString::fromUtf8("job was unsuccessful"));
        return;
    }

    RMM::Message m(j.message());

    switch (jobList_[j.id()].composeType()) {

        case EmpathComposeForm::Reply:      _reply(j.id(), m);      break; 
        case EmpathComposeForm::ReplyAll:   _reply(j.id(), m);      break;
        case EmpathComposeForm::Forward:    _forward(j.id(), m);    break; 
        case EmpathComposeForm::Bounce:     _bounce(j.id(), m);     break;
        case EmpathComposeForm::Normal:     default:                break;
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

    config->setGroup(QString::fromUtf8("Compose"));

    QStrList l;

    config->readListEntry(QString::fromUtf8("ExtraHeaders"), l, ',');

    // Standard headers
    l.prepend("Bcc");
    l.prepend("Cc");
    l.prepend("To");
    l.append("Subject");

    for (QStrListIterator it(l); it.current(); ++it)
        composeForm.setHeader(QString::fromUtf8(it.current()), QString::null);
}

   void
EmpathComposer::_reply(EmpathJobID id, RMM::Message message)
{
    EmpathComposeForm composeForm(jobList_[id]);
    QString to, cc;
    KConfig * config = new KConfig(QString::fromUtf8("emaildefaults"));

    referenceHeaders_ = _referenceHeaders(message);

    // First fill in the primary return address. This will be the Reply-To
    // address if there's one given, otherwise it will be the first in
    // the sender list.

    if (message.envelope().has(RMM::HeaderReplyTo)) 
        to = message.envelope().replyTo().at(0).asString();
    else if (message.envelope().has(RMM::HeaderFrom)) 
        to = message.envelope().from().at(0).asString();
    else
        to = i18n("Could not find sender of this message") +
            QString::fromUtf8(" <postmaster@localhost>");

    composeForm.setHeader(QString::fromUtf8("To"), to);

    if (composeForm.composeType() == EmpathComposeForm::ReplyAll) {

        if (message.envelope().has(RMM::HeaderCc)) 

            for (unsigned int i = 0; i < message.envelope().cc().count(); i++) {
                if (i > 0)
                    cc += QString::fromUtf8(", ");
                if (cc.length() > 70)
                    cc += QString::fromUtf8("\r\n ");
                cc +=
                    QString::fromUtf8(message.envelope().cc().at(i).asString());
            }

        config->setGroup(QString::fromUtf8("UserInfo"));

        RMM::Address me(config->readEntry(QString::fromUtf8("EmailAddress")).ascii());
        RMM::Address msgTo(message.envelope().to().at(0));

        if (me != msgTo && !cc.isEmpty()) 
            cc += QString::fromUtf8(message.envelope().to().asString());

        composeForm.setHeader(QString::fromUtf8("Cc"), cc);
    }

    // Fill in the subject.
    QString s = message.envelope().subject().asString();

    if (s.isEmpty())
        composeForm.setHeader(
                QString::fromUtf8("Subject"),
                i18n("Re: (no subject given)")
        );
    else
        if (s.find(QRegExp(QString::fromUtf8("^[Rr][Ee]:"))) != -1)
            composeForm.setHeader(QString::fromUtf8("Subject"), s);
        else
            composeForm.setHeader(QString::fromUtf8("Subject"),
                QString::fromUtf8("Re: ") + s);

    // Now quote original message if we need to.

    config->setGroup(QString::fromUtf8("Compose"));

    // Add the 'On (date) (name) wrote' bit

    if (config->readBoolEntry(QString::fromUtf8("AutoQuoteOnReply"), false)) {

        s = message.data();

        // Remove the signature
        int sigpos = s.find(QRegExp(QString::fromUtf8("\n--[ ]?\n")));
        if (sigpos != -1)
            s.truncate(sigpos);

        _quote(s); 

        QString thingyWrote;

        if (composeForm.composeType() == EmpathComposeForm::ReplyAll)
            thingyWrote =
                config->readEntry(QString::fromUtf8("PhraseReplyAll"));
        else
            thingyWrote =
                config->readEntry(QString::fromUtf8("PhraseReplySender"));

        // Be careful here. We don't want to reveal people's
        // email addresses.
        if (message.envelope().has(RMM::HeaderFrom) &&
            !message.envelope().from().at(0).phrase().isEmpty()) {

            thingyWrote.replace(QRegExp(QString::fromUtf8("\\%s")),
                message.envelope().from().at(0).phrase());

            if (message.envelope().has(RMM::HeaderDate))
                thingyWrote.replace(QRegExp(QString::fromUtf8("\\%d")),
                    message.envelope().date().qdt().date().toString());
            else
                thingyWrote.replace(QRegExp(QString::fromUtf8("\\%d")),
                    i18n("An unknown date and time"));

            composeForm.setBody('\n' + thingyWrote + '\n' + s);
        }
    }

    jobList_.replace(id, composeForm);

    delete config;
}

    void
EmpathComposer::_forward(EmpathJobID id, RMM::Message message)
{
    EmpathComposeForm composeForm(jobList_[id]);
    QString s;

    // Fill in the subject.
    s = message.envelope().subject().asString();

    if (s.isEmpty()) 
        composeForm.setHeader(
            QString::fromUtf8("Subject"),
            i18n("Fwd: (no subject)")
        );
    else
        if (s.find(QRegExp(QString::fromUtf8("^[Ff][Ww][Dd]:"))) != -1)
            composeForm.setHeader(QString::fromUtf8("Subject"), s);
        else
            composeForm.setHeader(
                QString::fromUtf8("Subject"),
                QString::fromUtf8("Fwd: ") + s
            );

    QString description = i18n("Forwarded message from %1");

    QString descriptionFilled =
        description.arg(message.envelope().firstSender().asString());

    message.setDescription(description.utf8());

    // TODO: Add setPreamble() to BodyPart
//    message.setPreamble(descriptionFilled.utf8())

    // TODO: Implement setDisposition() in BodyPart
//    message.setDisposition(RMM::DispositionTypeInline);

    // TODO: Add setDispositionFilename() to BodyPart
//    message.setDispositionFilename("message-" +
//        message.envelope().messageID().asString());

    message.setMimeGroup(RMM::MimeGroupMessage);
    message.setMimeValue(RMM::MimeValueRFC822);

    // Forward encoded as Base64. Wise ? XXX
    message.setEncoding(RMM::CteTypeBase64);

    composeForm.setBody(RMM::encodeBase64(message.asString()));

    jobList_.replace(id, composeForm);
}

    void
EmpathComposer::_bounce(EmpathJobID, RMM::Message /* m */)
{
    // TODO
}

    QString
EmpathComposer::_referenceHeaders(RMM::Message message)
{
    QString s;

    // Ok, here's the system.
    // Whatever happens, we create an In-Reply-To.

    s += "In-Reply-To: " + message.envelope().messageID().asString() + "\n";

    if (!message.envelope().has(RMM::HeaderReferences)) {
        s += "References: " + message.envelope().messageID().asString() + "\n";
        return s;
    }

    // There is a references header.

    QString references;

    QString refs(message.envelope().references().asString());

    QStrList l;

    RMM::tokenise(refs.utf8(), " ", l);

    // While there are more than 10 references, remove the one in
    // the second position. This way, we'll end up with 10 again,
    // and remove all the older references, excluding the oldest.

    while (l.count() > 9)
        l.remove(2);

    QStrListIterator it(l);

    for (; it.current(); ++it) {
        references += QString::fromUtf8(it.current());
        references += QString::fromUtf8(" ");
    }

    references += message.envelope().messageID().asString();

    s +=
        QString::fromUtf8("References: ") +
        references +
        QString::fromUtf8("\n");

    return s;
}

    QString
EmpathComposer::_stdHeaders()
{
    QString s;

    KConfig * config = new KConfig(QString::fromUtf8("emaildefaults"));
    config->setGroup(QString::fromUtf8("UserInfo"));

    QString fullName = config->readEntry(QString::fromUtf8("FullName"));
    QString emailAddress = config->readEntry(QString::fromUtf8("EmailAddress"));
    QString organization = config->readEntry(QString::fromUtf8("Organization"));

    if (fullName.isEmpty()) {
        empathDebug(QString::fromUtf8("Name of user is empty."));
    }

    if (emailAddress.isEmpty()) {

        // Shit !
        empathDebug(QString::fromUtf8("User's email address is empty !"));
        emailAddress = QString::fromUtf8("no.address.given@nowhere");
    }

    s +=
        QString::fromUtf8("From: ") +
        fullName +
        QString::fromUtf8(" <") + emailAddress + QString::fromUtf8(">\n");

    if (!organization.isEmpty())
        s +=
            QString::fromUtf8("Organization: ") +
            organization +
            QString::fromUtf8("\n");

    RMM::DateTime dt;
    dt.createDefault();
    s +=
        QString::fromUtf8("Date: ") +
        QString::fromUtf8(dt.asString()) +
        QString::fromUtf8("\n");

    RMM::MessageID id;
    id.createDefault();

    s +=
        QString::fromUtf8("Message-Id: ") +
        QString::fromUtf8(id.asString()) +
        QString::fromUtf8("\n");

    delete config;

    return s;
}

    QString
EmpathComposer::_signature()
{
    QString sig;

    KConfig * config(KGlobal::config());

    config->setGroup(QString::fromUtf8("Compose"));

    if (config->readBoolEntry(QString::fromUtf8("AddSignature"), false)) {

        QFile f(config->readEntry(QString::fromUtf8("Signature")));

        if (f.open(IO_ReadOnly)) {    
            QTextStream t(&f);
            QString s;
            t >> s;
            sig = QString::fromUtf8("\n-- \n") + s;
        }
    }

    return sig;
}

    void
EmpathComposer::_quote(QString & s)
{
    EmpathQuotedText quoted(s);
    quoted.rewrap(70);
    quoted.quote();
    s = quoted.asString();
}

// vim:ts=4:sw=4:tw=78
