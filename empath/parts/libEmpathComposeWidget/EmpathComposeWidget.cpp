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

// Qt includes
#include <qsplitter.h>
#include <qlayout.h>
#include <qmultilineedit.h>

// KDE includes
#include <kaction.h>
#include <klocale.h>
#include <kinstance.h>
#include <ktexteditor.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kservice.h>
#include <kdebug.h>

// Local includes
#include "EmpathComposeWidget.h"
#include "EmpathAttachmentListWidget.h"
#include "EmpathEnvelopeWidget.h"

extern "C"
{
    void *init_libEmpathComposePart()
    {
        return new EmpathComposePartFactory;
    }
}

KInstance * EmpathComposePartFactory::instance_ = 0L;

EmpathComposePartFactory::EmpathComposePartFactory()
{
    // Empty.
}

EmpathComposePartFactory::~EmpathComposePartFactory()
{
    delete instance_;
    instance_ = 0L;
}

    QObject *
EmpathComposePartFactory::create(
    QObject * parent,
    const char * name,
    const char *,
    const QStringList &
)
{
    QObject * o = new MyEmpathComposePart((QWidget *)parent, name);
    emit objectCreated(o);
    return o;
}

    KInstance *
EmpathComposePartFactory::instance()
{
    if (0 == instance_)
        instance_ = new KInstance("EmpathComposePart");

    return instance_;
}

// -------------------------------------------------------------------------

MyEmpathComposePart::MyEmpathComposePart(
    QWidget * parent,
    const char * name
)
    :   EmpathComposePart(parent, name)
{
    setInstance(EmpathComposePartFactory::instance());

    widget_ = new EmpathComposeWidget(parent);
    widget_->setFocusPolicy(QWidget::StrongFocus);
    setWidget(widget_);

    _initActions();

    setXMLFile("EmpathComposePart.rc");
}

MyEmpathComposePart::~MyEmpathComposePart()
{
    // Empty.
}

    void
MyEmpathComposePart::_initActions()
{
    new KAction
        (
         i18n("Send now"),
         "empath_message_send",
         0,
         widget_,
         SLOT(s_sendNow()),
         actionCollection(),
         "messageSendNow"
        );

    new KAction
        (
         i18n("Send later"),
         "empath_message_send_later",
         0,
         widget_,
         SLOT(s_sendLater()),
         actionCollection(),
         "messageSendLater"
        );

    new KAction
        (
         i18n("Postpone"),
         "empath_message_send_later",
         0,
         widget_,
         SLOT(s_postpone()),
         actionCollection(),
         "messagePostpone"
        );

    new KToggleAction
        (
         i18n("Confirm delivery"),
         "empath_confirm_delivery",
         0,
         widget_,
         SLOT(s_confirmDelivery()),
         actionCollection(),
         "messageConfirmDelivery"
        );

    new KToggleAction
        (
         i18n("Confirm reading"),
         "empath_confirm_read",
         0,
         widget_,
         SLOT(s_confirmDelivery()),
         actionCollection(),
         "messageConfirmReading"
        );

    new KToggleAction
        (
         i18n("Encrypt"),
         "empath_message_encrypt",
         0,
         widget_,
         SLOT(s_encrypt()),
         actionCollection(),
         "messageEncrypt"
        );

    new KToggleAction
        (
         i18n("Digitally sign"),
         "empath_message_digsign",
         0,
         widget_,
         SLOT(s_digitallySign()),
         actionCollection(),
         "messageDigitalSign"
        );

    new KToggleAction
        (
         i18n("Sign"),
         "empath_message_sign",
         0,
         widget_,
         SLOT(s_sign()),
         actionCollection(),
         "messageSign"
        );
}

    void
MyEmpathComposePart::setForm(const EmpathComposeForm & form)
{
    widget_->setForm(form);
}

EmpathComposeWidget::EmpathComposeWidget(QWidget * parent)
    :  
        QWidget(parent, "EmpathComposeWidget"),
        envelopeWidget_(0),
        editorPart_(0),
        editorView_(0),
        attachmentWidget_(0),
        actionCollection_(0)
{
    QSplitter * splitter = new QSplitter(Vertical, this, "splitter");
 
    envelopeWidget_ = new EmpathEnvelopeWidget(this);

    KTrader::OfferList offers =
        KTrader::self()->query("KTextEditor/Document");

    if (offers.count() >= 1)
    {
        KService::Ptr service = *offers.begin();

        KLibFactory * factory =
            KLibLoader::self()->factory(service->library().utf8());

        if (0 != factory)
        {
            editorPart_ = static_cast<KTextEditor::Document *>
                (factory->create(this, 0, "KTextEditor::Document"));

            if (0 != editorPart_)
            {
                editorView_ =
                    editorPart_->createView(splitter, "KTextEditor part");
            }
            else
            {
                kdError() << "Can't create a KTextEditor part" << endl;
            }
        }
        else
        {
            kdError() << "Can't find a KTextEditor factory" << endl;
        }
    }
    else
    {
        kdError() << "Can't find a KTextEditor library" << endl;
    }

    attachmentWidget_   = new EmpathAttachmentListWidget(splitter);

    attachmentWidget_->hide();

    QVBoxLayout * layout = new QVBoxLayout(this, 4);

    layout->addWidget(envelopeWidget_);
    layout->addWidget(splitter);

    _initActions();
}

EmpathComposeWidget::~EmpathComposeWidget()
{
    // Empty.
}

    void
EmpathComposeWidget::setForm(const EmpathComposeForm & form)
{
    composeForm_ = form;

    empathDebug("Setting headers.....................");

    envelopeWidget_->setHeaders(composeForm_.visibleHeaders());

    empathDebug(".................. headers set");

    editorPart_->setText(composeForm_.body());

    switch (composeForm_.composeType())
    {
        case EmpathComposeForm::Normal:
        case EmpathComposeForm::Reply:
        case EmpathComposeForm::ReplyAll:
            editorView_->setFocus();
            break;

        case EmpathComposeForm::Forward:
        default:
            envelopeWidget_->setFocus();
            break;
    }
}

    EmpathComposeForm
EmpathComposeWidget::form()
{
    composeForm_.setVisibleHeaders(envelopeWidget_->headers());
    composeForm_.setBody(editorPart_->text());
//TODO    composeForm_.attachments = attachmentWidget_.attachments();
    return composeForm_;
}

    void
EmpathComposeWidget::_initActions()
{
    actionCollection_ = new KActionCollection(this, "actionCollection");

    QValueList<KAction *> childActions( 
            attachmentWidget_->actionCollection()->actions());

    QValueListIterator<KAction *> it;
    for (it = childActions.begin(); it != childActions.end(); ++it)
        actionCollection_->insert(*it);
}

    void
EmpathComposeWidget::s_editorDone(bool, QCString)
{
    empathDebug("STUB");
}

    void
EmpathComposeWidget::s_sendNow()
{
    empathDebug("STUB");
}

    void
EmpathComposeWidget::s_sendLater()
{
    empathDebug("STUB");
}

    void
EmpathComposeWidget::s_postpone()
{
    empathDebug("STUB");
}

    void
EmpathComposeWidget::s_confirmDelivery()
{
    empathDebug("STUB");
}

    void
EmpathComposeWidget::s_encrypt()
{
    empathDebug("STUB");
}

    void
EmpathComposeWidget::s_digitallySign()
{
    empathDebug("STUB");
}

    void
EmpathComposeWidget::s_sign()
{
    empathDebug("STUB");
}

// vim:ts=4:sw=4:tw=78
#include "EmpathComposeWidget.moc"
