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

// Local includes
#include "EmpathEnvelopeWidget.h"
#include "EmpathHeaderSpecWidget.h"

EmpathEnvelopeWidget::EmpathEnvelopeWidget(QWidget * parent)
    :
        QVBox(parent, "EmpathEnvelopeWidget"),
        maxSizeColOne_(0)
{
    setSpacing(2);
}

EmpathEnvelopeWidget::~EmpathEnvelopeWidget()
{
    // Empty.
}

    void
EmpathEnvelopeWidget::setHeaders(const QMap<QString, QString> & envelope)
{
    QMap<QString, QString>::ConstIterator it(envelope.begin());

    for (; it != envelope.end(); ++it) 
        _addHeader(it.key(), it.data());

    _lineUpHeaders();

    setFocusProxy(headerSpecList_.getFirst());
}

    QMap<QString, QString>
EmpathEnvelopeWidget::headers()
{
    QMap<QString, QString> envelope;

    QListIterator<EmpathHeaderSpecWidget> it(headerSpecList_);
    
    for (; it.current(); ++it)
        envelope[it.current()->headerName()] = it.current()->headerBody();
    
    return envelope;
}

   bool
EmpathEnvelopeWidget::haveRecipient()
{
    QListIterator<EmpathHeaderSpecWidget> it(headerSpecList_);

    for (; it.current(); ++it)
        if (
            !it.current()->headerBody().isEmpty() &&
            (
                (0 == stricmp(it.current()->headerName().utf8(), "to"))     ||
                (0 == stricmp(it.current()->headerName().utf8(), "bcc"))    ||
                (0 == stricmp(it.current()->headerName().utf8(), "cc"))
            )
        )
        return true;
    
    return false;
}
    
    bool
EmpathEnvelopeWidget::haveSubject()
{
    QListIterator<EmpathHeaderSpecWidget> it(headerSpecList_);

    for (; it.current(); ++it)
        if (
            !it.current()->headerBody().isEmpty() &&
            (0 == stricmp(it.current()->headerName().utf8(), "subject"))
        )
        return true;
    
    return false;
}

    void
EmpathEnvelopeWidget::_addHeader(const QString & name, const QString & body) 
{
    EmpathHeaderSpecWidget * newHsw =
        new EmpathHeaderSpecWidget(name, body, this);
    
    if (!headerSpecList_.isEmpty()) {

        QObject::connect(
            newHsw,                     SIGNAL(goUp()),
            headerSpecList_.getLast(),  SLOT(s_setFocus()));

        QObject::connect(
            headerSpecList_.getLast(),  SIGNAL(goDown()),
            newHsw,                     SLOT(s_setFocus()));
    }
    
    headerSpecList_.append(newHsw); 
    maxSizeColOne_ = QMAX(newHsw->sizeOfColumnOne(), maxSizeColOne_);
    newHsw->show();
}
 
    void
EmpathEnvelopeWidget::_lineUpHeaders()
{
    QListIterator<EmpathHeaderSpecWidget> hit(headerSpecList_);
    
    for (; hit.current(); ++hit)
        hit.current()->setColumnOneSize(maxSizeColOne_);
}

// vim:ts=4:sw=4:tw=78
#include "EmpathEnvelopeWidget.moc"
