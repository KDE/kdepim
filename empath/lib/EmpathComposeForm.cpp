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
# pragma implementation "EmpathComposeForm.h"
#endif

#include "EmpathComposeForm.h"

EmpathComposeForm::EmpathComposeForm()
    :   composeType_(ComposeNormal)
{
    // Empty.
}

EmpathComposeForm::~EmpathComposeForm()
{
    // Empty.
}

EmpathComposeForm::EmpathComposeForm(const EmpathComposeForm & f)
    :   composeType_        (f.composeType_),
        visibleHeaders_     (f.visibleHeaders_),
        invisibleHeaders_   (f.invisibleHeaders_),
        body_               (f.body_),
        attachments_        (f.attachments_)
{
    // Empty.
}

    EmpathComposeForm &
EmpathComposeForm::operator = (const EmpathComposeForm & f)
{
    if (this == &f) // Avoid a = a.
        return *this;

    composeType_        = f.composeType_;
    visibleHeaders_     = f.visibleHeaders_;
    invisibleHeaders_   = f.invisibleHeaders_;
    body_               = f.body_;
    attachments_        = f.attachments_;
    return *this;
}

    ComposeType
EmpathComposeForm::composeType() const
{
    return composeType_;
}

    RMM::REnvelope
EmpathComposeForm::visibleHeaders() const
{
    return visibleHeaders_;
}

    RMM::REnvelope
EmpathComposeForm::invisibleHeaders() const
{
    return invisibleHeaders_;
}

    QCString
EmpathComposeForm::body() const
{
    return body_;
}

    EmpathAttachmentSpecList
EmpathComposeForm::attachmentList() const
{
    return attachments_;
}

    void
EmpathComposeForm::setComposeType(ComposeType t)
{
    composeType_ = t;
}

    void
EmpathComposeForm::setVisibleHeaders(RMM::REnvelope e)
{
    visibleHeaders_ = e;
}

    void
EmpathComposeForm::setInvisibleHeaders(RMM::REnvelope e)
{
    invisibleHeaders_ = e;
}

    void
EmpathComposeForm::setBody(const QCString & s)
{
    body_ = s;
}

    void
EmpathComposeForm::setAttachmentList(EmpathAttachmentSpecList asl)
{
    attachments_ = asl;
}

    void
EmpathComposeForm::addAttachment(EmpathAttachmentSpec s)
{
    attachments_.append(s);
}

    void
EmpathComposeForm::removeAttachment(EmpathAttachmentSpec s)
{
    attachments_.remove(s);
}

    void
EmpathComposeForm::addVisibleHeader(RMM::RHeader h)
{
    visibleHeaders_.addHeader(h);
}

    void
EmpathComposeForm::addInvisibleHeader(RMM::RHeader h)
{
    invisibleHeaders_.addHeader(h);
}

    void
EmpathComposeForm::setHeader(QCString name, QCString body, bool visible)
{
    if (visible)
        invisibleHeaders_.set(name, body);
    else
        visibleHeaders_.set(name, body);
}

// vim:ts=4:sw=4:tw=78
