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


#include "EmpathComposeForm.h"

EmpathComposeForm::EmpathComposeForm()
    :   composeType_(Normal)
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

    EmpathComposeForm::ComposeType
EmpathComposeForm::composeType() const
{
    return composeType_;
}

    QMap<QString, QString>
EmpathComposeForm::visibleHeaders() const
{
    return visibleHeaders_;
}

    QMap<QString, QString>
EmpathComposeForm::invisibleHeaders() const
{
    return invisibleHeaders_;
}

    QString
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
EmpathComposeForm::setVisibleHeaders(const QMap<QString, QString> & e)
{
    visibleHeaders_ = e;
}

    void
EmpathComposeForm::setInvisibleHeaders(const QMap<QString, QString> & e)
{
    invisibleHeaders_ = e;
}

    void
EmpathComposeForm::setBody(const QString & s)
{
    body_ = s;
}

    void
EmpathComposeForm::setAttachmentList(const EmpathAttachmentSpecList & asl)
{
    attachments_ = asl;
}

    void
EmpathComposeForm::addAttachment(const EmpathAttachmentSpec & s)
{
    attachments_.append(s);
}

    void
EmpathComposeForm::removeAttachment(const EmpathAttachmentSpec & s)
{
    attachments_.remove(s);
}

    void
EmpathComposeForm::addVisibleHeader(
    const QString & name,
    const QString & body
)
{
    visibleHeaders_[name] = body;
}

    void
EmpathComposeForm::addInvisibleHeader(
    const QString & name,
    const QString & body
)
{
    invisibleHeaders_[name] = body;
}

    void
EmpathComposeForm::setHeader(
    const QString & name,
    const QString & body,
    bool visible
)
{
    if (visible)
        visibleHeaders_[name] = body;
    else
        invisibleHeaders_[name] = body;
}

// vim:ts=4:sw=4:tw=78
