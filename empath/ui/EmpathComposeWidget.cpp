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
# pragma implementation "EmpathComposeWidget.h"
#endif

// Qt includes
#include <qregexp.h>
#include <qfile.h>
#include <qtextstream.h>

// KDE includes
#include <kprocess.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kapp.h>

// Local includes
#include "EmpathComposeWidget.h"
#include "EmpathAttachmentListWidget.h"
#include "EmpathAddressSelectionWidget.h"
#include "EmpathSubjectSpecWidget.h"
#include "EmpathEditorProcess.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathMailSender.h"
#include <RMM_DateTime.h>
#include <RMM_Mailbox.h>
#include <RMM_Address.h>
#include <RMM_Token.h>

EmpathComposeWidget::EmpathComposeWidget(
        QWidget     * parent,
        const char  * name)
    :
        QWidget(parent, name),
        url_("")
{
    // Empty.
}

EmpathComposeWidget::EmpathComposeWidget(
        Empath::ComposeType t,
        const EmpathURL     & m,
        QWidget             * parent,
        const char          * name)
    :
        QWidget(parent, name),
        composeType_(t),
        url_(m)
{
    // Empty.
}

EmpathComposeWidget::EmpathComposeWidget(
        const QString   & recipient,
        QWidget         * parent,
        const char      * name)
    :
        QWidget(parent, name),
        composeType_(Empath::ComposeNormal),
        url_(""),
        recipient_(recipient)
{
    // Empty.
}

    void
EmpathComposeWidget::_init()
{    
    maxSizeColOne_ = 0;
    invisibleHeaders_.setAutoDelete(true);
   
    editorWidget_ = new QMultiLineEdit(this, "editorWidget");
    attachmentWidget_ = new EmpathAttachmentListWidget(this);
    
    KConfig * c = KGlobal::config();
    
    using namespace EmpathConfig;

    c->setGroup(GROUP_COMPOSE);
    
    // If user wants us to wrap lines at a specific value, we can do that
    // in the editor to their specified width. This should give the user
    // a good idea of what their text will look like on the other end.
    //
    // If the user doesn't want us to wrap, we'll wrap text dynamically
    // in the editor anyway to make editing easier. We must not wrap the
    // actual text we send though.

    if (!c->readBoolEntry(C_WRAP_LINES, true))
        editorWidget_->setWordWrap(QMultiLineEdit::DynamicWrap);
    
    else {
        
        editorWidget_->setWordWrap(QMultiLineEdit::FixedColumnWrap);
        editorWidget_->setWrapColumnOrWidth(
            c->readUnsignedNumEntry(C_WRAP_COLUMN, 76));
    }
        
    c->setGroup(GROUP_DISPLAY);
    editorWidget_->setFont(c->readFontEntry(UI_FIXED_FONT));
 
    // Layouts.
    QVBoxLayout * layout    = new QVBoxLayout(this, 4);
    QHBoxLayout * topLayout = new QHBoxLayout;
    headerLayout_           = new QVBoxLayout;

    topLayout->addLayout(headerLayout_,     1);
    topLayout->addWidget(attachmentWidget_, 0);

    layout->addLayout(topLayout,    0);
    layout->addWidget(editorWidget_,1);

    _addHeader("To");
    _addHeader("Cc");
    _addHeader("Bcc");
    _addExtraHeaders();
    _addHeader("Subject");

    _lineUpHeaders();

    switch (composeType_) {

        case Empath::ComposeReply:      _reply();       break; 
        case Empath::ComposeReplyAll:   _reply(true);   break; 
        case Empath::ComposeForward:    _forward();     break; 
        case Empath::ComposeNormal:      default:       break;
    }
    
    if (composeType_ == Empath::ComposeForward) {
        // Don't bother opening external editor
        return;
    }
    
    if (!recipient_.isEmpty())
        _set("To", recipient_);
    
    KGlobal::config()->setGroup(EmpathConfig::GROUP_COMPOSE);

    if (KGlobal::config()->readBoolEntry(EmpathConfig::C_USE_EXT_EDIT, false)) {

        editorWidget_->setEnabled(false);
        _spawnExternalEditor(editorWidget_->text().latin1());
    }
    
    headerSpecList_.first()->setFocus();
}

EmpathComposeWidget::~EmpathComposeWidget()
{
    empathDebug("dtor");
}

    RMM::RMessage
EmpathComposeWidget::message()
{
    empathDebug("message() called");

    RMM::RMessage msg(_envelope() + "\n" + _body());

    return msg;
}

    bool
EmpathComposeWidget::messageHasAttachments()
{
    empathDebug("messageHasAttachments() called");
    return false;
}

    void
EmpathComposeWidget::_spawnExternalEditor(const QCString & text)
{
    empathDebug("spawnExternalEditor() called");
    
    EmpathEditorProcess * p = new EmpathEditorProcess(text);
    CHECK_PTR(p);
    
    QObject::connect(
        p,         SIGNAL    (done            (bool, QCString)),
        this,    SLOT    (s_editorDone    (bool, QCString)));
    
    p->go();
}

    void
EmpathComposeWidget::_reply(bool toAll)
{
    RMM::RMessage * m(empath->message(url_));
    if (m == 0) return;
    
    RMM::RMessage message(*m);

    empath->finishedWithMessage(url_);
    
    QCString to, cc;
    
    // First fill in the primary return address. This will be the Reply-To
    // address if there's one given, otherwise it will be the first in
    // the sender list.
    if (!toAll) {
        
        empathDebug("Normal reply");

        if (message.envelope().has(RMM::HeaderReplyTo)) {
            empathDebug("Has header reply to");
            to = message.envelope().replyTo().at(0)->asString();
        }
        else
        {
            empathDebug("Does not have header reply to");
            to = message.envelope().to().at(0)->asString();
        }
        
        empathDebug("to == " + to);
        
        _set("To", QString::fromLatin1(to));
    }
    
    if (toAll) {
        
        if (message.envelope().has(RMM::HeaderReplyTo)) {
        
            to = message.envelope().replyTo().asString();
            _set("To", QString::fromLatin1(to));
        
        } else if (message.envelope().has(RMM::HeaderFrom)) {
        
            to = message.envelope().from().at(0).asString();
            _set("To", QString::fromLatin1(to));
        }
    
    
        if (message.envelope().has(RMM::HeaderCc)) {

            if (message.envelope().cc().count() != 0) {
            
                bool firstTime = false;
                
                for (int i = 0; i < message.envelope().cc().count(); i++) {
            
                    if (!firstTime)
                        cc += ", ";
                    cc += message.envelope().cc().at(i).asString();
                }

            }
        }
    
        KConfig c(KGlobal::dirs()->findResource("config", "kcmemailrc"), true);
        c.setGroup("UserInfo");
        RMM::RAddress me(c.readEntry("EmailAddress").latin1());
        
        if (!(me == *(message.envelope().to().at(0))))
            if (!cc.isEmpty())
                cc += message.envelope().to().asString();
        
        _set("Cc", QString::fromLatin1(cc));
    }
    
    // Fill in the subject.
    QString s = QString::fromLatin1(message.envelope().subject().asString());
    empathDebug("Subject was \"" + s + "\""); 

    if (s.isEmpty())
        _set("Subject",    i18n("Re: (no subject given)"));
    else
        if (s.find(QRegExp("^[Rr][Ee]:")) != -1)
            _set("Subject", s);
        else
            _set("Subject", "Re: " + s);

    // Now quote original message if we need to.
    
    KConfig * c(KGlobal::config());
    c->setGroup(EmpathConfig::GROUP_COMPOSE);
    
    empathDebug("Quoting original if necessary");

    // Add the 'On (date) (name) wrote' bit
        
    if (!c->readBoolEntry(EmpathConfig::C_AUTO_QUOTE)) {
        editorWidget_->setFocus();
        return;
    }

    s = message.data();

    s.replace(QRegExp("\\n"), "\n> ");

    QString thingyWrote =
        c->readEntry(EmpathConfig::C_PHRASE_REPLY_SENDER, "") + '\n';
    
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
    
        editorWidget_->setText('\n' + thingyWrote + s);
    }

    editorWidget_->setFocus();
}

    void
EmpathComposeWidget::_forward()
{
    empathDebug("Forwarding");
    
    RMM::RMessage * m(empath->message(url_));
    if (m == 0) return;
    
    RMM::RMessage message(*m);
    
    QString s;

    // Fill in the subject.
    s = QString::fromLatin1(message.envelope().subject().asString());
    empathDebug("Subject was \"" + s + "\""); 

    if (s.isEmpty())
        _set("Subject", i18n("Fwd: (no subject given)"));
    else
        if (s.find(QRegExp("^[Ff][Ww][Dd]:")) != -1)
            _set("Subject", s);
        else
            _set("Subject", "Fwd: " + s);
}

    void
EmpathComposeWidget::s_editorDone(bool ok, QCString text)
{
    if (!ok) {
//        statusBar()->message(i18n("Message not modified by external editor"));
        return;
    }
    
    editorWidget_->setText(text);
    editorWidget_->setEnabled(true);
}

    void
EmpathComposeWidget::bugReport()
{
    _set("To", "rik@kde.org");
    _addInvisibleHeader("X-EmpathInfo:",
        "Empath Version: " + EMPATH_VERSION_STRING +
        " KDE Version: " + QString::fromLatin1(KDE_VERSION_STRING) +
        " Qt Version: " + QString::fromLatin1(QT_VERSION_STR));
    _lineUpHeaders();
    QString errorStr_;
    errorStr_ = "- " + i18n(
        "What were you trying to do when the problem occured ?");
    errorStr_ += "\n\n\n\n";
    errorStr_ += "- " + i18n(
        "What actually happened ?");
    errorStr_ += "\n\n\n\n";
    errorStr_ += "- " + i18n(
        "Exactly what did you do that caused the problem to manifest itself ?");
    errorStr_ += "\n\n\n\n";
    errorStr_ += "- " + i18n(
        "Do you have a suggestion as to how this behaviour can be corrected ?");
    errorStr_ += "\n\n\n\n";
    errorStr_ += "- " + i18n(
        "If you saw an error message, please try to reproduce it here.");
    editorWidget_->setText(errorStr_);
}

    void
EmpathComposeWidget::_addExtraHeaders()
{
    // Now check which headers we're supposed to enable the editing of.
    KConfig * c(KGlobal::config());
    c->setGroup(EmpathConfig::GROUP_COMPOSE);
    
    QStrList l;
    c->readListEntry(EmpathConfig::C_EXTRA_HEADERS, l, ',');
    empathDebug("There are " + QString().setNum(l.count()) + " headers");
    
    QStrListIterator it(l);
    
    for (; it.current(); ++it)
        _addHeader(it.current());
}        
    
    void
EmpathComposeWidget::_addHeader(const QString & n, const QString & b)
{
    EmpathHeaderSpecWidget * newHsw = new EmpathHeaderSpecWidget(n, b, this);
    newHsw->show();
    headerLayout_->addWidget(newHsw);

    if (!headerSpecList_.isEmpty()) {
        QObject::connect(
            newHsw, SIGNAL(lineUp()),
            headerSpecList_.getLast(), SLOT(setFocus()));
        QObject::connect(
            headerSpecList_.getLast(), SIGNAL(lineDown()),
            newHsw, SLOT(setFocus()));
    }
    
    headerSpecList_.append(newHsw); 
    maxSizeColOne_ = QMAX(newHsw->sizeOfColumnOne(), maxSizeColOne_);
}

    void
EmpathComposeWidget::_addInvisibleHeader(const QString & n, const QString & b)
{
    RMM::RHeader * h =
        new RMM::RHeader(QCString(n.latin1()) + ": " + QCString(b.latin1()));
    invisibleHeaders_.append(h);
}

    QString
EmpathComposeWidget::_get(const QString & headerName)
{
    QListIterator<EmpathHeaderSpecWidget> it(headerSpecList_);
    
    QCString n(headerName.latin1() + ':');
    
    for (; it.current(); ++it)
        if (stricmp(it.current()->headerName().latin1(), n) == 0) {
            return it.current()->headerBody();
        }
    
    return QString::null;
}

    void
EmpathComposeWidget::_set(const QString & headerName, const QString & val)
{
    QListIterator<EmpathHeaderSpecWidget> it(headerSpecList_);
    
    QCString n(headerName.latin1());
    n += ':';
    empathDebug("Setting \"" + QString(n) + "\"");
    empathDebug("Val = " + val);
    
    for (; it.current(); ++it) {
        empathDebug("looking at \"" + it.current()->headerName() + "\"");
        if (stricmp(it.current()->headerName().latin1(), n) == 0) {
            it.current()->setHeaderBody(val);
            return;
        }
    }
    
    _addHeader(headerName, val);
}

    void
EmpathComposeWidget::s_cut()
{
    editorWidget_->cut();
}

    void
EmpathComposeWidget::s_copy()
{
    editorWidget_->copy();
}

    void
EmpathComposeWidget::s_paste()
{
    editorWidget_->paste();
}

    void
EmpathComposeWidget::s_selectAll()
{
    editorWidget_->selectAll();
}

    bool
EmpathComposeWidget::haveTo()
{
    QListIterator<EmpathHeaderSpecWidget> it(headerSpecList_);

    for (; it.current(); ++it)
        if (stricmp(it.current()->headerName().latin1(), "To:") == 0 &&
            !it.current()->headerBody().isEmpty())
            return true;
    
    return false;
}
    
    bool
EmpathComposeWidget::haveSubject()
{
    QListIterator<EmpathHeaderSpecWidget> it(headerSpecList_);

    for (; it.current(); ++it)
        if (stricmp(it.current()->headerName().latin1(), "Subject:") == 0 &&
            !it.current()->headerBody().isEmpty())
            return true;
    
    return false;
}

    void
EmpathComposeWidget::s_addAttachment()
{
    attachmentWidget_->addAttachment();
}

    void
EmpathComposeWidget::s_editAttachment()
{
    attachmentWidget_->editAttachment();
}

    void
EmpathComposeWidget::s_removeAttachment()
{
    attachmentWidget_->removeAttachment();
}

    void
EmpathComposeWidget::_lineUpHeaders()
{
    QListIterator<EmpathHeaderSpecWidget> hit(headerSpecList_);
    
    for (; hit.current(); ++hit)
        hit.current()->setColumnOneSize(maxSizeColOne_);
}

    QCString
EmpathComposeWidget::_envelope()
{
    QCString s;
    
    s += _referenceHeaders();
    s += _stdHeaders();
    s += _visibleHeaders();
    s += _invisibleHeaders();
    
    return s;
}

    QCString
EmpathComposeWidget::_body()
{
    QCString s = editorWidget_->text().latin1();
    
    KConfig * c(KGlobal::config());

    c->setGroup(EmpathConfig::GROUP_COMPOSE);
    
    if (!c->readBoolEntry(EmpathConfig::C_ADD_SIG))
        return s;

    QFile f(c->readEntry(EmpathConfig::C_SIG_PATH));
    
    if (f.open(IO_ReadOnly)) {    

        QTextStream t(&f);
    
        QCString sig;
    
        while (!t.eof())
            sig += t.readLine().latin1() + QCString("\n");
    
        s += "\n" + sig;
    }

    return s;
}

    QCString
EmpathComposeWidget::_referenceHeaders()
{
    QCString s;

    if (composeType_ != Empath::ComposeReply    &&
        composeType_ != Empath::ComposeReplyAll    )
        return s;

    RMM::RMessage * m(empath->message(url_));
    
    if (m == 0) {
        empathDebug("No message to reply to");
        return s;
    }
    
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
EmpathComposeWidget::_visibleHeaders()
{
    QCString s;
    
    QListIterator<EmpathHeaderSpecWidget> it(headerSpecList_);
    
    for (; it.current(); ++it) {
        
        if (it.current()->headerBody().isEmpty())
            continue;
        
        s += it.current()->headerName().latin1();
        s += " ";
        s += it.current()->headerBody().latin1();
        s += "\n";
    }
    
    return s;
}
    
    QCString
EmpathComposeWidget::_invisibleHeaders()
{
    QCString s;
    
    RMM::RHeaderListIterator it(invisibleHeaders_);
    
    for (; it.current(); ++it) {
        s += it.current()->headerName();
        s += " ";
        s += it.current()->headerBody()->asString();
        s += "\n";
    }
    
    return s;
}

    QCString
EmpathComposeWidget::_stdHeaders()
{
    QCString s;
    
    KConfig c(KGlobal::dirs()->findResource("config", "kcmemailrc"), true);
    c.setGroup("UserInfo");

    s += QCString("From: ");
    s += QCString(c.readEntry("FullName").latin1());
    s += QCString(" <");
    s += QCString(c.readEntry("EmailAddress").latin1());
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

// vim:ts=4:sw=4:tw=78
